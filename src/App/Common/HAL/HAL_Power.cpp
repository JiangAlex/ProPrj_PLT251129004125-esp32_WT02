#include "HAL.h"
#include <Arduino.h>

// 分壓比: Vout = Vin * (200k / (9k + 200k))
constexpr float R1 = 9000.0f;   // 9kΩ
constexpr float R2 = 200000.0f; // 200kΩ
constexpr float VOLTAGE_DIVIDER = (R2 / (R1 + R2));
constexpr float ADC_MAX = 4095.0f;
constexpr float REF_VOLTAGE = 3.3f;

static Power_Info_t powerInfo = {3.7f, 85, false}; // 預設值

void HAL::Power_Init() {
    // 初始化 ADC
    analogReadResolution(12); // 12-bit
    // 若用 ESP-IDF 可用 adc1_config_width, adc1_config_channel_atten
    Serial.println("Power module initialized.");
}

static float HAL::readBatteryVoltage() {
    // Check if battery detection pin is configured
    if (CONFIG_BAT_DET_PIN < 0) {
        return 3.7f; // Return default voltage if pin not configured
    }
    
    int raw = analogRead(CONFIG_BAT_DET_PIN);
    float vout = (raw / ADC_MAX) * REF_VOLTAGE;
    float vin = vout / VOLTAGE_DIVIDER;
    return vin;
}

static int HAL::voltageToPercent(float vin) {
    // 假設鋰電池 3.0V=0%, 4.2V=100%
    if (vin < 3.0f) return 0;
    if (vin > 4.2f) return 100;
    return (int)((vin - 3.0f) * 100.0f / (4.2f - 3.0f));
}

void HAL::Power_Update()
{
    // 讀取電池電壓並更新資訊
    float vin = readBatteryVoltage();
    int percent = voltageToPercent(vin);
    powerInfo.voltage = vin;
    powerInfo.percent = percent;
    // charging 狀態可根據 GPIO 或 PMIC 判斷，這裡預設 false
    powerInfo.isCharging = false;
}

void HAL::Power_GetInfo(Power_Info_t* info) {
    if (!info) return;
    float vin = readBatteryVoltage();
    int percent = voltageToPercent(vin);
    // charging 狀態可根據 GPIO 或 PMIC 判斷，這裡預設 false
    info->voltage = vin;
    info->percent = percent;
    info->isCharging = false;
    powerInfo = *info;
}

int HAL::Power_GetPercent() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.percent;
}

float HAL::Power_GetVoltage() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.voltage;
}

bool HAL::Power_IsCharging() {
    Power_Info_t info;
    Power_GetInfo(&info);
    return info.isCharging;
}
