#include "HAL.h"
#include <TinyGPSPlus.h>

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

void HAL::GPS_Update() {
    if (!gps_initialized) return;
    while (Serial1.available()) {
        gps.encode(Serial1.read());
    }
}

bool HAL::GPS_GetInfo(GPS_Info_t *info) {
    if (!info) return false;
    info->latitude = gps.location.lat();
    info->longitude = gps.location.lng();
    info->altitude = gps.altitude.meters();
    info->speed = gps.speed.kmph();
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
