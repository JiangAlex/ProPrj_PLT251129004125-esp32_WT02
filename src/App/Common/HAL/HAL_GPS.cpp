#include "HAL.h"
#include <TinyGPSPlus.h>
#include <time.h>
#include <sys/time.h>

#define GPS_RX_PIN  14
#define GPS_TX_PIN  27
#define GPS_BAUD    9600

static TinyGPSPlus gps;
static bool gps_initialized = false;

void HAL::GPS_Init() {
    Serial1.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    gps_initialized = true;
    Serial.printf("[GPS] Init: UART1 RX=%d TX=%d @ %d\n", GPS_RX_PIN, GPS_TX_PIN, GPS_BAUD);
}

static bool gps_time_synced = false;

void HAL::GPS_Update() {
    if (!gps_initialized) return;
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }

    // Sync system clock from GPS time (once, when valid)
    if (!gps_time_synced && !HAL::Clock_IsSynced()
        && gps.time.isValid() && gps.date.isValid() 
        && gps.date.year() >= 2025 && gps.date.year() <= 2030
        && gps.location.isValid()) {
        struct tm t = {};
        t.tm_year = gps.date.year() - 1900;
        t.tm_mon = gps.date.month() - 1;
        t.tm_mday = gps.date.day();
        t.tm_hour = gps.time.hour();
        t.tm_min = gps.time.minute();
        t.tm_sec = gps.time.second();
        // Use mktime then subtract timezone offset to get true UTC epoch
        // Or set TZ to UTC temporarily
        setenv("TZ", "UTC0", 1);
        tzset();
        time_t epoch = mktime(&t);
        // Restore timezone
        int8_t tz = HAL::Clock_GetTimezone();
        char tzStr[16];
        snprintf(tzStr, sizeof(tzStr), "UTC%+d", -tz);
        setenv("TZ", tzStr, 1);
        tzset();

        struct timeval tv = { .tv_sec = epoch };
        settimeofday(&tv, NULL);
        gps_time_synced = true;
        Serial.printf("[GPS] Time synced: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
                      gps.date.year(), gps.date.month(), gps.date.day(),
                      gps.time.hour(), gps.time.minute(), gps.time.second());
    }
}

bool HAL::GPS_GetInfo(GPS_Info_t *info) {
    if (!info) return false;
    info->latitude = gps.location.lat();
    info->longitude = gps.location.lng();
    info->altitude = gps.altitude.meters();
    info->speed = gps.speed.kmph();
    info->course = gps.course.isValid() ? gps.course.deg() : 0.0f;
    info->satellites = gps.satellites.value();
    info->isValid = gps.location.isValid();
    return info->isValid;
}

bool HAL::GPS_LocationIsValid() {
    return gps.location.isValid();
}

double HAL::GPS_GetDistanceOffset(GPS_Info_t *info, double preLong, double preLat) {
    if (!info || !info->isValid) return 0.0;
    return TinyGPSPlus::distanceBetween(info->latitude, info->longitude, preLat, preLong);
}
