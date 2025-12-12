#ifndef __HAL_DEF_H
#define __HAL_DEF_H

#include <stdint.h>

/* SA818 Power Mode Enum (Forward declaration from SA818_Channels.h) */
enum SA818_PowerMode {
    SA818_LOW_POWER = 0,   // L-CH: 409.75-409.9875 MHz
    SA818_HIGH_POWER = 1   // H-CH: 430.1375-439.4375 MHz
};

/* SA818 Info Structure */
typedef struct {
    uint8_t channel;
    float freq_rx;
    float freq_tx;
    uint8_t squelch;
    uint8_t volume;
    uint8_t ctcss_rx;
    uint8_t ctcss_tx;
    uint8_t BW;
    uint8_t SQ;
} SA818_Info_t;

/* SA818 Channel Info Structure */
typedef struct {
    int channel;                    // 頻道號碼 (1-20)
    SA818_PowerMode powerMode;      // 功率模式 (L-CH/H-CH)
    float frequency;                // 當前頻率 (MHz)
    char powerModeName[8];          // 功率模式名稱 ("L-CH" 或 "H-CH")
    char frequencyRange[32];        // 頻率範圍描述
} SA818_ChannelInfo_t;

/* Power Info Structure */
typedef struct {
    float voltage;        // 電池電壓
    uint8_t percent;   // 電池百分比
    bool isCharging;      // 是否充電中
    bool isLowBattery;
    uint32_t uptime;
} Power_Info_t;

/* Clock Info Structure */
typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    uint8_t week;
} Clock_Info_t;

/* GPS Info Structure */
typedef struct {
    double latitude;
    double longitude;
    double altitude;
    float speed;
    uint8_t satellites;
    bool isValid;
} GPS_Info_t;

/* IMU Info Structure */
typedef struct {
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
    float temp;
} IMU_Info_t;

/* Sport Status Info Structure */
typedef struct {
    uint32_t steps;
    float distance;
    uint32_t calories;
    uint32_t activeTime;
} SportStatus_Info_t;

/* MAG Info Structure */
typedef struct {
    float x;
    float y;
    float z;
    float heading;
} MAG_Info_t;

/* WiFi Info Structure */
typedef struct {
    bool apEnabled;
    bool staEnabled;
    bool staConnected;
    char ssid[32];
    int8_t rssi;
    char ipAddress[16];
} WiFi_Info_t;

/* BLE Info Structure */
typedef struct {
    bool isEnabled;
    bool isConnected;
    char deviceName[32];
} BLE_Info_t;

/* Smart Assistant Info Structure */
typedef struct {
    bool isEnabled;
    bool isListening;
    char lastCommand[64];
} Saa_Info_t;

/* Display Info Structure */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t brightness;
    bool isOn;
} U8g2_Info_t;

/* Button Event Types */
typedef enum {
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESS,
    BUTTON_EVENT_RELEASE,
    BUTTON_EVENT_HOLD,
    BUTTON_EVENT_DOUBLE
} Button_Event_t;

/* Button Info Structure */
typedef struct {
    uint8_t pin;
    Button_Event_t event;
    bool isPressed;
    unsigned long pressTime;
} Button_Info_t;

#endif // __HAL_DEF_H