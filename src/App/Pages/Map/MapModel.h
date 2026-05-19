#ifndef MAP_MODEL_H
#define MAP_MODEL_H

#include "App/Common/HAL/HAL.h"
#include <Arduino.h>

namespace Page
{
    #define MAX_TRACK_POINTS 500
    #define MAX_WAYPOINTS 50
    #define WPT_NAME_LEN 12

    struct TrackPoint { float lat; float lon; };
    struct Waypoint { float lat; float lon; char name[WPT_NAME_LEN]; };

    class MapModel
    {
    public:
        void Init();
        void Deinit();
        void Update();

        // GPS
        bool GetGPSInfo(GPS_Info_t *info);
        bool IsGPSValid();

        // Track data
        bool LoadFromSPIFFS();
        int GetTrackCount() { return trackCount; }
        TrackPoint* GetTrackPoints() { return trackPoints; }
        int GetWaypointCount() { return wptCount; }
        Waypoint* GetWaypoints() { return waypoints; }

        // Navigation
        int GetTargetWpt() { return targetWptIdx; }
        void SetTargetWpt(int idx);
        float GetDistanceToTarget(); // meters
        float GetBearingToTarget();  // degrees

    private:
        TrackPoint* trackPoints = nullptr;
        Waypoint* waypoints = nullptr;
        int trackCount = 0;
        int wptCount = 0;
        int targetWptIdx = 0;
        GPS_Info_t lastGPS;
    };
}

#endif
