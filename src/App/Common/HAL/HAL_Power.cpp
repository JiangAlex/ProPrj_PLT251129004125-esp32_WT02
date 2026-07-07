#include "HAL.h"
#include <Arduino.h>

// 分壓比: Vout = Vin * R2 / (R1 + R2) = Vin * 0.5
// VBAT ──(75kΩ R1)── IO39 ──(75kΩ R2)── GND
constexpr float R1 = 75000.0f;  // 75kΩ (上臂，接 VBAT)
constexpr float R2 = 75000.0f;  // 75kΩ (下臂，接 GND)
constexpr float VOLTAGE_DIVIDER = (R2 / (R1 + R2)); // 0.5
constexpr float ADC_MAX = 4095.0f;
constexpr float REF_VOLTAGE = 3.3f;

static Power_Info_t powerInfo = {3.7f, 85, false}; // 預設值

void HAL::Power_Init() {
    // 初始化 ADC
    analogReadResolution(12); // 12-bit
    
    // 充電狀態偵測 pin (TP4056 CHRG#: LOW = charging, HIGH = not charging)
    if (CONFIG_BAT_CHG_DET_PIN >= 0) {
        pinMode(CONFIG_BAT_CHG_DET_PIN, INPUT_PULLUP);
    }
    
    Serial.printf("Power_Init: BAT_ADC=GPIO%d, CHG_DET=GPIO%d\n", 
                  CONFIG_BAT_DET_PIN, CONFIG_BAT_CHG_DET_PIN);
    Serial.println("Power module initialized.");
    
    // 立即執行一次更新，避免使用預設值
    Power_Update();
}

static float readBatteryVoltage() {
    // Check if battery detection pin is configured
    if (CONFIG_BAT_DET_PIN < 0) {
        return 3.7f; // Return default voltage if pin not configured
    }
    
    // Multi-sample for stability
    int raw = 0;
    for (int i = 0; i < 8; i++) {
        raw += analogRead(CONFIG_BAT_DET_PIN);
        delayMicroseconds(100);
    }
    raw /= 8;
    
    float vout = (raw / ADC_MAX) * REF_VOLTAGE;
    float vin = vout / VOLTAGE_DIVIDER;  // Recover actual VBAT
    
    // ESP32 ADC 線性校正（實測 VBAT=3.189V 時 ADC 算出 3.024V，偏差 +0.165V）
    vin += 0.165f;
    
    // Debug: 印出 ADC raw 值（僅在 Power_Init 時印一次）
    static bool firstRead = true;
    if (firstRead) {
        firstRead = false;
        Serial.printf("[Power] ADC raw=%d, Vout=%.3fV, Vin(cal)=%.3fV\n", raw, vout, vin);
    }
    
    return vin;
}

static int voltageToPercent(float vin) {
    // 鋰電池 3.0V=0%, 4.2V=100%
    if (vin < 3.0f) return 0;
    if (vin > 4.2f) return 100;
    return (int)((vin - 3.0f) * 100.0f / (4.2f - 3.0f));
}

static bool readChargingState() {
    // TP4056 CHRG# pin: LOW = charging, HIGH = full or not charging
    if (CONFIG_BAT_CHG_DET_PIN < 0) {
        return false;
    }
    
    // 多次取樣去彈跳：5 次中取多數決
    int lowCount = 0;
    for (int i = 0; i < 5; i++) {
        if (digitalRead(CONFIG_BAT_CHG_DET_PIN) == LOW) {
            lowCount++;
        }
        delayMicroseconds(200);
    }
    
    // 加入遲滯：狀態切換需要明確的多數（4/5 以上）
    static bool lastState = false;
    if (lowCount >= 4) {
        lastState = true;
    } else if (lowCount <= 1) {
        lastState = false;
    }
    // 2~3 次時維持上次狀態（遲滯區間）
    
    return lastState;
}

void HAL::Power_Update()
{
    float vin = readBatteryVoltage();
    int percent = voltageToPercent(vin);
    bool charging = readChargingState();

    powerInfo.voltage = vin;
    powerInfo.percent = percent;
    powerInfo.isCharging = charging;
}

void HAL::Power_GetInfo(Power_Info_t* info) {
    if (!info) return;
    // 使用已快取的 powerInfo（由 Power_Update 每 500ms 更新）
    *info = powerInfo;
}

int HAL::Power_GetPercent() {
    return powerInfo.percent;
}

float HAL::Power_GetVoltage() {
    return powerInfo.voltage;
}

bool HAL::Power_IsCharging() {
    return powerInfo.isCharging;
}
