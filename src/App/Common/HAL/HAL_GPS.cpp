#include "HAL.h"
#include <TinyGPSPlus.h>
#include "App/Configs/pinout.h"

static TinyGPSPlus gps;
static bool gps_initialized = false;

void HAL::GPS_Init() {
    Serial2.begin(SERIAL_GPS_BAUD, SERIAL_8N1, SERIAL_GPS_RXPIN, SERIAL_GPS_TXPIN);
    gps_initialized = true;
    Serial.println("[GPS] Init: UART2 RX=16 TX=17 @ 9600");
}

void HAL::GPS_Update() {
    if (!gps_initialized) return;
    while (Serial2.available()) {
        gps.encode(Serial2.read());
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
