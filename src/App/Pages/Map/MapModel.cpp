#include "MapModel.h"
#include "App/Pages/Trekking/TrekkingModel.h"
#include <SPIFFS.h>
#include <math.h>

using namespace Page;

#define DEG_TO_RAD 0.017453292519943295

void MapModel::Init()
{
    trackPoints = (TrackPoint*)malloc(MAX_TRACK_POINTS * sizeof(TrackPoint));
    waypoints = (Waypoint*)malloc(MAX_WAYPOINTS * sizeof(Waypoint));
    trackCount = 0;
    wptCount = 0;
    targetWptIdx = 0;
    memset(&lastGPS, 0, sizeof(lastGPS));
    LoadFromSPIFFS();
}

void MapModel::Deinit()
{
    if (trackPoints) { free(trackPoints); trackPoints = nullptr; }
    if (waypoints) { free(waypoints); waypoints = nullptr; }
}

void MapModel::Update()
{
    HAL::GPS_GetInfo(&lastGPS);
}

bool MapModel::GetGPSInfo(GPS_Info_t *info)
{
    *info = lastGPS;
    return lastGPS.isValid;
}

bool MapModel::IsGPSValid() { return lastGPS.isValid; }

bool MapModel::LoadFromSPIFFS()
{
    String path = TrekkingModel::activeGPXPath;
    // Fallback: try first file in index
    if (path.length() == 0) {
        File idx = SPIFFS.open("/gpx/index.txt", "r");
        if (idx) {
            String line = idx.readStringUntil('\n');
            line.trim();
            idx.close();
            if (line.length() > 0) {
                int num = line.toInt();
                char buf[32];
                snprintf(buf, sizeof(buf), "/gpx/%03d.bin", num);
                path = String(buf);
            }
        }
    }
    if (path.length() == 0 || !SPIFFS.exists(path)) return false;

    File f = SPIFFS.open(path, "r");
    if (!f) return false;

    // Header: uint16_t trackCount, uint16_t wptCount
    uint16_t tc, wc;
    f.read((uint8_t*)&tc, 2);
    f.read((uint8_t*)&wc, 2);

    trackCount = min((int)tc, MAX_TRACK_POINTS);
    wptCount = min((int)wc, MAX_WAYPOINTS);

    f.read((uint8_t*)trackPoints, trackCount * sizeof(TrackPoint));
    f.read((uint8_t*)waypoints, wptCount * sizeof(Waypoint));
    f.close();

    Serial.printf("[Map] Loaded %d track pts, %d waypoints\n", trackCount, wptCount);
    return true;
}

void MapModel::SetTargetWpt(int idx)
{
    if (wptCount == 0) return;
    if (idx < 0) idx = wptCount - 1;
    if (idx >= wptCount) idx = 0;
    targetWptIdx = idx;
}

float MapModel::GetDistanceToTarget()
{
    if (!lastGPS.isValid || wptCount == 0) return 0;
    Waypoint &wp = waypoints[targetWptIdx];
    // Haversine
    double dLat = (wp.lat - lastGPS.latitude) * DEG_TO_RAD;
    double dLon = (wp.lon - lastGPS.longitude) * DEG_TO_RAD;
    double a = sin(dLat/2)*sin(dLat/2) +
               cos(lastGPS.latitude*DEG_TO_RAD)*cos(wp.lat*DEG_TO_RAD)*
               sin(dLon/2)*sin(dLon/2);
    double c = 2*atan2(sqrt(a), sqrt(1-a));
    return 6371000.0 * c; // meters
}

float MapModel::GetBearingToTarget()
{
    if (!lastGPS.isValid || wptCount == 0) return 0;
    Waypoint &wp = waypoints[targetWptIdx];
    double lat1 = lastGPS.latitude * DEG_TO_RAD;
    double lat2 = wp.lat * DEG_TO_RAD;
    double dLon = (wp.lon - lastGPS.longitude) * DEG_TO_RAD;
    double y = sin(dLon) * cos(lat2);
    double x = cos(lat1)*sin(lat2) - sin(lat1)*cos(lat2)*cos(dLon);
    double brng = atan2(y, x) * 180.0 / M_PI;
    return fmod(brng + 360.0, 360.0);
}
