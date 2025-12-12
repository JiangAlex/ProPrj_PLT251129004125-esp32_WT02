#include "HAL.h"
#include <time.h>
#include <sys/time.h>

static bool clock_initialized = false;

void HAL::Clock_Init()
{
    Serial.println("Initializing clock...");
    
    // 設定時區為台灣時區 (UTC+8)
    setenv("TZ", "CST-8", 1);
    tzset();
    
    clock_initialized = true;
    Serial.println("Clock initialized with Taiwan timezone (UTC+8)");
}

void HAL::Clock_GetInfo(::Clock_Info_t *info)
{
    if (!info || !clock_initialized) return;
    
    struct tm timeinfo;
    time_t now = time(nullptr);
    localtime_r(&now, &timeinfo);
    
    // 檢查時間是否有效
    if (timeinfo.tm_year < 125 || timeinfo.tm_year > 200) {
        // 時間無效，使用預設時間
        info->hour = 14;
        info->minute = 30;
        info->second = 0;
        info->day = 12;
        info->month = 11;
        info->year = 2025;
        info->week = 2; // Tuesday
    } else {
        info->hour = timeinfo.tm_hour;
        info->minute = timeinfo.tm_min;
        info->second = timeinfo.tm_sec;
        info->day = timeinfo.tm_mday;
        info->month = timeinfo.tm_mon + 1;
        info->year = timeinfo.tm_year + 1900;
        info->week = timeinfo.tm_wday;
    }
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