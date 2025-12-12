#ifndef SA818_CHANNELS_H
#define SA818_CHANNELS_H

#include "HAL_Def.h"  // 使用 HAL_Def.h 中的 SA818_PowerMode 定義

// SA818 對講機頻道配置
// 支援 High Power (H-CH) 和 Low Power (L-CH) 兩種功率模式

// 頻道數量定義
#define SA818_MAX_CHANNELS 20
#define SA818_MIN_CHANNEL 1
#define SA818_MAX_CHANNEL 20

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
    return (powerMode == SA818_LOW_POWER) ? 
           "409.75-409.9875 MHz" : 
           "430.1375-439.4375 MHz";
}

#endif // SA818_CHANNELS_H