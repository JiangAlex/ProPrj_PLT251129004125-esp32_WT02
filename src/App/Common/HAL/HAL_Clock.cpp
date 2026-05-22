#include "HAL.h"
#include <time.h>
#include <sys/time.h>
#include <WiFi.h>
#include <Preferences.h>

static bool clock_initialized = false;
static bool clock_synced = false;
static bool clock_24h = true;
static int8_t clock_timezone = 8;
static uint8_t screen_timeout = 10;

// NTP 伺服器設定
static const char* ntpServer1 = "time.stdtime.gov.tw";  // 台灣標準時間
static const char* ntpServer2 = "pool.ntp.org";
static const long gmtOffset_sec = 8 * 3600;  // UTC+8 (台灣)
static const int daylightOffset_sec = 0;     // 台灣無夏令時間

void HAL::Clock_Init()
{
    Serial.println("Initializing clock...");
    
    // Load saved settings
    Preferences prefs;
    prefs.begin("system", true);
    clock_timezone = prefs.getChar("tz", 8);
    clock_24h = prefs.getBool("24h", true);
    screen_timeout = prefs.getUChar("scrto", 10);
    prefs.end();
    
    // Apply saved timezone
    char tzStr[16];
    snprintf(tzStr, sizeof(tzStr), "UTC%+d", -clock_timezone);
    setenv("TZ", tzStr, 1);
    tzset();
    
    clock_initialized = true;
    Serial.printf("Clock initialized: TZ=UTC%+d, %s\n", clock_timezone, clock_24h ? "24H" : "12H");
}

void HAL::Clock_GetInfo(::Clock_Info_t *info)
{
    if (!info) return;
    
    // 設置預設值（防止返回垃圾值）
    info->hour = 12;
    info->minute = 0;
    info->second = 0;
    info->day = 18;
    info->month = 12;
    info->year = 2025;
    info->week = 4; // Thursday
    
    if (!clock_initialized) {
        // 時鐘未初始化，返回預設值
        return;
    }
    
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    
    // 檢查時間是否有效 (tm_year 是從 1900 起算，125 = 2025 年)
    if (timeinfo.tm_year >= 124 && timeinfo.tm_year <= 200) {
        // 時間有效，使用系統時間
        info->hour = timeinfo.tm_hour;
        info->minute = timeinfo.tm_min;
        info->second = timeinfo.tm_sec;
        info->day = timeinfo.tm_mday;
        info->month = timeinfo.tm_mon + 1;
        info->year = timeinfo.tm_year + 1900;
        info->week = timeinfo.tm_wday;
    }
    // 否則保持預設值
}

void HAL::Clock_SetInfo(const ::Clock_Info_t *info)
{
    if (!info || !clock_initialized) return;
    
    struct tm timeinfo;
    timeinfo.tm_hour = info->hour;
    timeinfo.tm_min = info->minute;
    timeinfo.tm_sec = info->second;
    timeinfo.tm_mday = info->day;
    timeinfo.tm_mon = info->month - 1;
    timeinfo.tm_year = info->year - 1900;
    timeinfo.tm_wday = info->week;
    
    time_t t = mktime(&timeinfo);
    struct timeval tv = { .tv_sec = t };
    settimeofday(&tv, NULL);
}

const char *HAL::Clock_GetWeekString(uint8_t week)
{
    static const char* weekStrings[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", 
        "Thursday", "Friday", "Saturday"
    };
    
    if (week > 6) return "Unknown";
    return weekStrings[week];
}

void HAL::Clock_GetTimeString(char* timeStr, char* battStr)
{
    if (!timeStr || !battStr || !clock_initialized) return;
    
    ::Clock_Info_t clockInfo;
    Clock_GetInfo(&clockInfo);
    
    // 格式化時間 (24小時制，台灣習慣)
    snprintf(timeStr, 16, "%02d:%02d", clockInfo.hour, clockInfo.minute);
    
    // 模擬電池電量 (85-99%)
    int battPercent = 85 + (millis() / 10000) % 15;
    snprintf(battStr, 16, "Batt:%d%%", battPercent);
}

bool HAL::Clock_SyncNTP()
{
    if (!clock_initialized) {
        Serial.println("[Clock] Not initialized, cannot sync NTP");
        return false;
    }
    
    // 檢查 WiFi 連接狀態
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Clock] WiFi not connected, cannot sync NTP");
        return false;
    }
    
    Serial.println("[Clock] Starting NTP sync...");
    Serial.printf("[Clock] NTP servers: %s, %s\n", ntpServer1, ntpServer2);
    
    // 配置 NTP
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
    
    // 等待時間同步（最多 10 秒）
    struct tm timeinfo;
    int retry = 0;
    const int maxRetry = 10;
    
    while (!getLocalTime(&timeinfo) && retry < maxRetry) {
        Serial.printf("[Clock] Waiting for NTP sync... (%d/%d)\n", retry + 1, maxRetry);
        delay(1000);
        retry++;
    }
    
    if (retry >= maxRetry) {
        Serial.println("[Clock] NTP sync timeout!");
        return false;
    }
    
    clock_synced = true;
    
    // 打印同步後的時間
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("[Clock] NTP sync success! Time: %s (UTC+8)\n", buf);
    
    return true;
}

bool HAL::Clock_IsSynced()
{
    return clock_synced;
}

void HAL::Clock_Set24Hour(bool en) {
    clock_24h = en;
    Preferences prefs;
    prefs.begin("system", false);
    prefs.putBool("24h", en);
    prefs.end();
}
bool HAL::Clock_Is24Hour() { return clock_24h; }

void HAL::Clock_SetTimezone(int8_t tz) {
    if (tz < -12) tz = -12;
    if (tz > 14) tz = 14;
    clock_timezone = tz;
    char tzStr[16];
    snprintf(tzStr, sizeof(tzStr), "UTC%+d", -tz);
    setenv("TZ", tzStr, 1);
    tzset();
    Preferences prefs;
    prefs.begin("system", false);
    prefs.putChar("tz", tz);
    prefs.end();
}

int8_t HAL::Clock_GetTimezone() { return clock_timezone; }

void HAL::Clock_SetScreenTimeout(uint8_t sec) {
    screen_timeout = sec;
    Preferences prefs;
    prefs.begin("system", false);
    prefs.putUChar("scrto", sec);
    prefs.end();
}

uint8_t HAL::Clock_GetScreenTimeout() { return screen_timeout; }