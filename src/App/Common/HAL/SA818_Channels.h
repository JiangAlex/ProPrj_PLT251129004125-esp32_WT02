#ifndef SA818_CHANNELS_H
#define SA818_CHANNELS_H

#include "HAL_Def.h"  // 使用 HAL_Def.h 中的 SA818_PowerMode 定義

// SA818 對講機頻道配置
// 支援 High Power (H-CH) 和 Low Power (L-CH) 兩種功率模式

// 頻道數量定義
#define SA818_MAX_CHANNELS 20
#define SA818_MIN_CHANNEL 1
#define SA818_MAX_CHANNEL 20

// Default to VHF (0) if not defined, or if explicitly set to 0
#if !defined(DRA818_CONFIG_UHF) || (DRA818_CONFIG_UHF == 0)

// VHF Low Power 頻道配置 (L-CH) - Example frequencies starting at 144.8000
static const float SA818_L_CH_FREQUENCIES[SA818_MAX_CHANNELS] = {
    144.8000, 144.8125, 144.8250, 144.8375, 144.8500,
    144.8625, 144.8750, 144.8875, 144.9000, 144.9125,
    144.9250, 144.9375, 144.9500, 144.9625, 144.9750,
    144.9875, 145.0000, 145.0125, 145.0250, 145.0375
};

// VHF High Power 頻道配置 (H-CH) - Example frequencies
static const float SA818_H_CH_FREQUENCIES[SA818_MAX_CHANNELS] = {
    154.1000, 154.1125, 154.1250, 154.1375, 154.1500,
    154.1625, 154.1750, 154.1875, 154.2000, 154.2125,
    154.2250, 154.2375, 154.2500, 154.2625, 154.2750,
    154.2875, 154.3000, 154.3125, 154.3250, 154.3375
};

#else

// Low Power 頻道配置 (L-CH)
static const float SA818_L_CH_FREQUENCIES[SA818_MAX_CHANNELS] = {
    409.7500,  // Channel 1
    409.7625,  // Channel 2
    409.7750,  // Channel 3
    409.7875,  // Channel 4
    409.8000,  // Channel 5
    409.8125,  // Channel 6
    409.8250,  // Channel 7
    409.8375,  // Channel 8
    409.8500,  // Channel 9
    409.8625,  // Channel 10
    409.8750,  // Channel 11
    409.8875,  // Channel 12
    409.9000,  // Channel 13
    409.9125,  // Channel 14
    409.9250,  // Channel 15
    409.9375,  // Channel 16
    409.9500,  // Channel 17
    409.9625,  // Channel 18
    409.9750,  // Channel 19
    409.9875   // Channel 20
};

// High Power 頻道配置 (H-CH)
static const float SA818_H_CH_FREQUENCIES[SA818_MAX_CHANNELS] = {
    430.1375,  // Channel 1
    430.4375,  // Channel 2
    431.1375,  // Channel 3
    431.4375,  // Channel 4
    432.1375,  // Channel 5
    432.4375,  // Channel 6
    433.1375,  // Channel 7
    433.4375,  // Channel 8
    434.1375,  // Channel 9
    434.4375,  // Channel 10
    435.1375,  // Channel 11
    435.4375,  // Channel 12
    436.1375,  // Channel 13
    436.4375,  // Channel 14
    437.1375,  // Channel 15
    437.4375,  // Channel 16
    438.1375,  // Channel 17
    438.4375,  // Channel 18
    439.1375,  // Channel 19
    439.4375   // Channel 20
};

#endif

// 輔助函數：根據功率模式和頻道號獲取頻率
inline float getSA818Frequency(SA818_PowerMode powerMode, int channel) {
    if (channel < SA818_MIN_CHANNEL || channel > SA818_MAX_CHANNEL) {
        return 0.0; // 無效頻道
    }
    
    int index = channel - 1; // 轉換為陣列索引 (0-19)
    
    if (powerMode == SA818_LOW_POWER) {
        return SA818_L_CH_FREQUENCIES[index];
    } else {
        return SA818_H_CH_FREQUENCIES[index];
    }
}

// 輔助函數：獲取功率模式名稱
inline const char* getPowerModeName(SA818_PowerMode powerMode) {
    return (powerMode == SA818_LOW_POWER) ? "L-CH" : "H-CH";
}

// 輔助函數：獲取頻率範圍描述
inline const char* getFrequencyRange(SA818_PowerMode powerMode) {
#if !defined(DRA818_CONFIG_UHF) || (DRA818_CONFIG_UHF == 0)
    return (powerMode == SA818_LOW_POWER) ? 
           "144.80-145.0375 MHz" : 
           "145.10-145.3375 MHz";
#else
    return (powerMode == SA818_LOW_POWER) ? 
           "409.75-409.9875 MHz" : 
           "430.1375-439.4375 MHz";
#endif
}

#endif // SA818_CHANNELS_H