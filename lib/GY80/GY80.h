#pragma once

#include <Arduino.h>
#include <Wire.h>

// Lightweight GY-80 helper for BMP085/BMP180 (pressure/temp), HMC5883L (mag), ADXL345 (accel), ITG3200/MPU6050 (gyro)
// This is a minimal, single-file helper intended for quick integration on ESP32.

namespace GY80 {

struct BMPData {
    bool ok;
    float temperature; // C
    float pressure;    // hPa
};

struct AccelData {
    bool ok;
    float ax; // g
    float ay;
    float az;
};

struct MagData {
    bool ok;
    float mx; // uT (raw units, approximate)
    float my;
    float mz;
};

struct GyroData {
    bool ok;
    float gx; // deg/s (raw approximate)
    float gy;
    float gz;
};

class GY80 {
public:
    GY80();
    ~GY80();

    // Call in setup(); returns true if Wire initialized
    bool begin(TwoWire &wire = Wire, uint8_t sda = SDA, uint8_t scl = SCL, uint32_t freq = 400000);

    // Minimal reads (non-blocking for simplicity)
    BMPData readBMP();
    AccelData readAccel();
    MagData readMag();
    GyroData readGyro();

private:
    TwoWire *_wire;
    uint8_t _sda_pin;
    uint8_t _scl_pin;
    uint32_t _freq;

    // helpers
    bool probeDevice(uint8_t addr);
    bool writeRegister(uint8_t addr, uint8_t reg, uint8_t value);
    bool readRegisters(uint8_t addr, uint8_t reg, uint8_t *buf, size_t len);
};

} // namespace GY80
