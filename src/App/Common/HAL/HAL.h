/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __HAL_H
#define __HAL_H

#include <stdint.h>
#include "HAL_Def.h"
#include "CommonMacro.h"
#include "App/Configs/Config.h"
#include <freertos/FreeRTOS.h>
//#include <freertos/task.h>
#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;

namespace HAL
{
    typedef bool (*CommitFunc_t)(void *info, void *userData);
    //typedef void (*Display_CallbackFunc_t)(void);

    void HAL_Init(void);
    void HAL_Update(void);

    /* Backlight */
    void Backlight_Init();
    uint16_t Backlight_GetValue();
    void Backlight_SetValue(int16_t val);
    void Backlight_SetGradual(uint16_t target, uint16_t time = 500);
    void Backlight_ForceLit(bool en);

    /* FaultHandle */
    void FaultHandle_Init();

    /* I2C */
    void I2C_Init(bool startScan);
    void I2C_Deinit();
    bool I2C_IsDeviceConnected(uint8_t address);
    bool I2C_ReadBytes(uint8_t deviceAddress, uint8_t registerAddress, uint8_t* data, size_t length);
    bool I2C_WriteBytes(uint8_t deviceAddress, uint8_t registerAddress, const uint8_t* data, size_t length);
    bool I2C_WriteByte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t data);
    bool I2C_ReadByte(uint8_t deviceAddress, uint8_t registerAddress, uint8_t &data);
    bool I2C_ScanForDevice(uint8_t targetAddress);
    void I2C_DisplayConnectedDevices();
    void I2C_SetClock(uint32_t frequency);
    uint32_t I2C_GetClock();
    bool I2C_SetPins(int sdaPin, int sclPin);
    bool I2C_ProbeDevice(uint8_t deviceAddress);
    bool I2C_ResetBus();
    bool I2C_ScanAndDisplayDevices();
    bool I2C_TestReadWrite(uint8_t deviceAddress, uint8_t testRegister, uint8_t testValue);
    void I2C_DisplayBusInfo();
    void I2C_GetPins(int &sdaPin, int &sclPin);

    /* IMU */
    void IMU_Init(void);
    void IMU_SetCommitCallback(CommitFunc_t func, void *userData);
    void IMU_Update();

    /* MAG */
    bool MAG_Init();
    void MAG_SetCommitCallback(CommitFunc_t func, void *userData);
    void MAG_Update();
    void MAG_GetInfo(::MAG_Info_t *info);
    void MAG_SetCalibration(void);

    /* SD */

    /* Power */
    void Power_Init();
    static float readBatteryVoltage();
    static int voltageToPercent(float vin);
    void Power_Update();
    void Power_GetInfo(Power_Info_t* info);
    int Power_GetPercent();
    float Power_GetVoltage();
    bool Power_IsCharging();


    /* Clock */
    void Clock_Init();
    void Clock_GetInfo(::Clock_Info_t *info);
    void Clock_SetInfo(const ::Clock_Info_t *info);
    const char *Clock_GetWeekString(uint8_t week);
    void Clock_GetTimeString(char* timeStr, char* battStr);

    /* GPS */
    void GPS_Init();
    void GPS_Update();
    bool GPS_GetInfo(::GPS_Info_t *info);
    bool GPS_LocationIsValid();
    double GPS_GetDistanceOffset(::GPS_Info_t *info, double preLong, double preLat);

    /* Buzzer */

    /* Button */
    void Button_Init();
    void Button_Update();
    ::Button_Event_t Button_GetEvent(uint8_t buttonIndex);
    bool Button_IsPressed(uint8_t buttonIndex);
    bool Button_IsHold(uint8_t buttonIndex);
    void Button_GetInfo(uint8_t buttonIndex, ::Button_Info_t *info);
    void Button_HandleEvents();
    
    /* Button Index */
    enum ButtonIndex {
        BUTTON_MENU_OK = 0,   // GPIO 32
        BUTTON_UP_BACK = 1,   // GPIO 33
        BUTTON_DOWN_FN = 2    // GPIO 34
    };

    /* Encoder */

    /* Audio */
    void Audio_Init();
    void Audio_Update();
    bool Audio_PlayMusic(const char *name);

    /* Memory */
    void Memory_DumpInfo();

    /* ENV */
    void ENV_Init(void);
    void ENV_SetCommitCallback(CommitFunc_t func, void *userData);
    void ENV_Update(void);

    /* WiFi */
    void WiFi_Init(void);
    void WiFi_Update(void);
    void WiFi_APSetEnable(bool en);
    void WiFi_STASetEnable(bool en);
    void WiFi_GetInfo(::WiFi_Info_t *info);

    /* OTA */
    void OTA_Init();

    /* BLE */
    void BLE_Init(void);
    void BLE_Update(void);
    void BLE_SetBegin(void);
    void BLE_SetSleep(void);
    void BLE_GetInfo(::BLE_Info_t *info);
    void BLE_SetKeyboardValue(uint8_t *c);
    void BLE_SetMouseMoveValue(uint8_t *c);
    void BLE_SetMouseClickValue(uint16_t b);

    /* ARPS-ESP */
    void MicroAPRS_Init();
    void MicroAPRS_Update();
    static void taskAPRS(void *pvParameters);

    /* SA818 */
    void SA818_Init();
    void SA818_scan();
    void SA818_GetInfo(::SA818_Info_t* info);
    
    /* SA818 Control Pins */
    void SA818_SetPowerDown(bool powerDown);    // Control PD pin
    void SA818_SetHighLowPower(bool highPower); // Control H/L pin
    bool SA818_GetPowerDown();                  // Read PD pin state
    bool SA818_GetHighLowPower();               // Read H/L pin state
    
    /* SA818 Channel Management */
    bool SA818_SetChannel(int channel, SA818_PowerMode powerMode);
    int SA818_GetChannel();
    SA818_PowerMode SA818_GetPowerMode();
    float SA818_GetCurrentFrequency();
    bool SA818_NextChannel();
    bool SA818_PreviousChannel();
    bool SA818_TogglePowerMode();
    void SA818_GetChannelInfo(::SA818_ChannelInfo_t* info);

    /* SA818 Volume */
    void SA818_SetVolume(uint8_t vol);
    uint8_t SA818_GetVolume();
    
    /* PTT (Push To Talk) */
    void PTT_Init();
    bool PTT_IsPressed();
    void PTT_SetTransmit(bool enable);

    /* U8g2 */
    void U8g2_Init();
    void U8g2_Test();
    void U8g2_Clear();
    void U8g2_SetBrightness(uint8_t brightness);
    void U8g2_Sleep();
    void U8g2_Wakeup();
    void U8g2_GetInfo(::U8g2_Info_t *info);
    //void U8g2_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    void U8g2_SendPixels(const uint16_t *pixels, uint32_t len);
}

#endif
