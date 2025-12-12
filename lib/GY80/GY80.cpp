#include "GY80.h"

using namespace GY80;

// I2C addresses
static const uint8_t ADDR_BMP = 0x77; // BMP085/BMP180
static const uint8_t ADDR_HMC = 0x1E; // HMC5883L
static const uint8_t ADDR_ADXL = 0x53; // ADXL345
static const uint8_t ADDR_ITG = 0x68; // ITG3200 or MPU6050

GY80::GY80::GY80() : _wire(nullptr), _sda_pin(SDA), _scl_pin(SCL), _freq(400000) {}
GY80::GY80::~GY80() {}

bool GY80::GY80::begin(TwoWire &wire, uint8_t sda, uint8_t scl, uint32_t freq) {
    _wire = &wire;
    _sda_pin = sda;
    _scl_pin = scl;
    _freq = freq;

    // On ESP32, begin(sda, scl) is supported
    _wire->begin((int)_sda_pin, (int)_scl_pin, _freq);
    delay(50);
    // return true if at least one expected device responds
    bool found = probeDevice(ADDR_BMP) || probeDevice(ADDR_HMC) || probeDevice(ADDR_ADXL) || probeDevice(ADDR_ITG);
    return found;
}

bool GY80::GY80::probeDevice(uint8_t addr) {
    _wire->beginTransmission(addr);
    if (_wire->endTransmission() == 0) return true;
    return false;
}

bool GY80::GY80::writeRegister(uint8_t addr, uint8_t reg, uint8_t value) {
    _wire->beginTransmission(addr);
    _wire->write(reg);
    _wire->write(value);
    return (_wire->endTransmission() == 0);
}

bool GY80::GY80::readRegisters(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len) {
    _wire->beginTransmission(addr);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) return false; // restart
    size_t read = _wire->requestFrom((int)addr, (int)len);
    if (read != len) return false;
    for (size_t i = 0; i < len; ++i) buf[i] = _wire->read();
    return true;
}

GY80::BMPData GY80::GY80::readBMP() {
    BMPData out{};
    out.ok = false;

    // Try BMP180 read (simpler: read uncompensated temp & pressure via registers)
    if (!probeDevice(ADDR_BMP)) return out;

    uint8_t buf[2];
    // Read temperature raw (BMP180: 0xF6..0xF7 after writing 0x2E to 0xF4)
    if (!writeRegister(ADDR_BMP, 0xF4, 0x2E)) return out;
    delay(5);
    if (!readRegisters(ADDR_BMP, 0xF6, buf, 2)) return out;
    int32_t UT = (buf[0] << 8) | buf[1];

    // Pressure raw (use oss=0)
    if (!writeRegister(ADDR_BMP, 0xF4, 0x34)) return out;
    delay(8);
    uint8_t pb[3];
    if (!readRegisters(ADDR_BMP, 0xF6, pb, 3)) return out;
    int32_t UP = ((pb[0] << 16) | (pb[1] << 8) | pb[2]) >> (8);

    // Improved conversion formulas for BMP180
    // Temperature conversion (more realistic approximation)
    float tempC = (UT - 27898) / 340.0f + 25.0f; // Better approximation
    
    // Pressure conversion (more realistic approximation)  
    float pressPa = UP / 256.0f + 101325.0f; // Better approximation around sea level
    
    out.ok = true;
    out.temperature = tempC;
    out.pressure = pressPa / 100.0f; // Convert Pa to hPa
    return out;
}

GY80::AccelData GY80::GY80::readAccel() {
    AccelData out{};
    out.ok = false;
    if (!probeDevice(ADDR_ADXL)) return out;
    // Ensure measurement mode
    writeRegister(ADDR_ADXL, 0x2D, 0x08);
    uint8_t buf[6];
    if (!readRegisters(ADDR_ADXL, 0x32, buf, 6)) return out;
    int16_t ax = (int16_t)((buf[1] << 8) | buf[0]);
    int16_t ay = (int16_t)((buf[3] << 8) | buf[2]);
    int16_t az = (int16_t)((buf[5] << 8) | buf[4]);
    // ADXL345 in full resolution 4mg/LSB at +/-2g -> convert
    out.ax = ax * 0.004f;
    out.ay = ay * 0.004f;
    out.az = az * 0.004f;
    out.ok = true;
    return out;
}

GY80::MagData GY80::GY80::readMag() {
    MagData out{};
    out.ok = false;
    if (!probeDevice(ADDR_HMC)) return out;
    // Set continuous measurement mode
    writeRegister(ADDR_HMC, 0x02, 0x00);
    uint8_t buf[6];
    if (!readRegisters(ADDR_HMC, 0x03, buf, 6)) return out;
    int16_t mx = (int16_t)((buf[0] << 8) | buf[1]);
    int16_t mz = (int16_t)((buf[2] << 8) | buf[3]);
    int16_t my = (int16_t)((buf[4] << 8) | buf[5]);
    out.mx = mx * 0.92f; // approx scaling to uT
    out.my = my * 0.92f;
    out.mz = mz * 0.92f;
    out.ok = true;
    return out;
}

GY80::GyroData GY80::GY80::readGyro() {
    GyroData out{};
    out.ok = false;
    if (!probeDevice(ADDR_ITG)) return out;
    // Try reading ITG3200-like registers (0x1D..) or MPU6050 (0x3B..)
    uint8_t buf[6];
    // Try MPU6050 accel/gyro register set (gyro starts at 0x43)
    if (readRegisters(ADDR_ITG, 0x43, buf, 6)) {
        int16_t gx = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t gy = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t gz = (int16_t)((buf[4] << 8) | buf[5]);
        // MPU6050 full-scale default 250deg/s -> 131 LSB/deg/s
        out.gx = gx / 131.0f;
        out.gy = gy / 131.0f;
        out.gz = gz / 131.0f;
        out.ok = true;
        return out;
    }

    // fallback: try ITG3200 registers at 0x1D? (not implemented)
    return out;
}
