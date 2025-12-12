#include "HAL.h"
#include "Wire.h"

void HAL::I2C_Init(bool startScan)
{
    Wire.begin(CONFIG_MCU_SDA_PIN, CONFIG_MCU_SCL_PIN);

    if(!startScan)
        return;

    uint8_t error, address;
    int nDevices;

    Serial.println("I2C: device scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++ )
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C: device found at address 0x"); 
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("I2C: unknow error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    Serial.printf("I2C: %d devices was found\r\n", nDevices);
}
void HAL::I2C_Deinit()
{
    Wire.end();
}
bool HAL::I2C_IsDeviceConnected(uint8_t address)
{
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    return (error == 0);
}
bool HAL::I2C_ReadBytes(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* data, size_t length)
{
    Wire.beginTransmission(deviceAddress);
    Wire.write(registerAddress);
    if (Wire.endTransmission(false) != 0) {
        return false; // Transmission error
    }

    Wire.requestFrom(deviceAddress, static_cast<uint8_t>(length));
    for (size_t i = 0; i < length && Wire.available(); i++) {
        data[i] = Wire.read();
    }

    return true;
}
bool HAL::I2C_WriteBytes(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t length)
{
    Wire.beginTransmission(deviceAddress);
    Wire.write(registerAddress);
    Wire.write(data, length);
    return (Wire.endTransmission() == 0);
}
bool HAL::I2C_WriteByte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t data)
{
    return HAL::I2C_WriteBytes(deviceAddress, registerAddress, &data, 1);
}
bool HAL::I2C_ReadByte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t &data)
{
    return HAL::I2C_ReadBytes(deviceAddress, registerAddress, &data, 1);
}
bool HAL::I2C_ScanForDevice(uint8_t targetAddress)
{
    Serial.printf("I2C_ScanForDevice: Scanning for device at address 0x%02X...\n", targetAddress);
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("  Found device at address 0x%02X\n", address);
            if (address == targetAddress) {
                Serial.printf("I2C_ScanForDevice: Target device found at address 0x%02X!\n", targetAddress);
                return true;
            }
        }
    }
    Serial.printf("I2C_ScanForDevice: Target device at address 0x%02X not found.\n", targetAddress);
    return false;
}
void HAL::I2C_DisplayConnectedDevices()
{
    Serial.println("I2C_DisplayConnectedDevices: Scanning for I2C devices...");
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("  Found I2C device at address 0x%02X\n", address);
        }
    }
}
void HAL::I2C_SetClock(uint32_t frequency)
{
    Wire.setClock(frequency);
}
uint32_t HAL::I2C_GetClock()
{
    // Note: Wire library does not provide a direct method to get the clock frequency
    // This function assumes that the frequency is known or stored elsewhere
    // For demonstration, we return a default value
    uint32_t I2C_Clock = Wire.getClock();
    return I2C_Clock; // Default to 100kHz
}
bool HAL::I2C_SetPins(int sdaPin, int sclPin)
{
    // Reinitialize Wire with new pins
    Wire.end();
    return Wire.begin(sdaPin, sclPin);
}

bool HAL::I2C_ProbeDevice(uint8_t deviceAddress)
{
    Wire.beginTransmission(deviceAddress);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
        Serial.printf("I2C_ProbeDevice: Device found at address 0x%02X\n", deviceAddress);
        return true;
    } else {
        Serial.printf("I2C_ProbeDevice: No device at address 0x%02X\n", deviceAddress);
        return false;
    }
}
bool HAL::I2C_ResetBus()
{
    // Note: Wire library does not provide a direct method to reset the I2C bus
    // This function is a placeholder to maintain API consistency
    // For demonstration, we reinitialize the Wire library
    Wire.end();
    return Wire.begin(CONFIG_MCU_SDA_PIN, CONFIG_MCU_SCL_PIN);
}
bool HAL::I2C_ScanAndDisplayDevices()
{
    Serial.println("I2C_ScanAndDisplayDevices: Scanning for I2C devices...");
    bool foundAny = false;
    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        uint8_t error = Wire.endTransmission();
        if (error == 0) {
            Serial.printf("  Found I2C device at address 0x%02X\n", address);
            foundAny = true;
        }
    }
    if (!foundAny) {
        Serial.println("I2C_ScanAndDisplayDevices: No I2C devices found.");
    }
    return foundAny;
}
bool HAL::I2C_TestReadWrite(uint8_t deviceAddress, uint8_t testRegister, uint8_t testValue)
{
    // Write test value
    if (!HAL::I2C_WriteByte(deviceAddress, testRegister, testValue)) {
        Serial.printf("I2C_TestReadWrite: Failed to write to device 0x%02X\n", deviceAddress);
        return false;
    }

    // Read back value
    uint8_t readValue = 0;
    if (!HAL::I2C_ReadByte(deviceAddress, testRegister, readValue)) {
        Serial.printf("I2C_TestReadWrite: Failed to read from device 0x%02X\n", deviceAddress);
        return false;
    }

    // Verify
    if (readValue != testValue) {
        Serial.printf("I2C_TestReadWrite: Mismatch! Wrote 0x%02X but read 0x%02X from device 0x%02X\n",
                      testValue, readValue, deviceAddress);
        return false;
    }

    Serial.printf("I2C_TestReadWrite: Read/Write test successful for device 0x%02X\n", deviceAddress);
    return true;
}

