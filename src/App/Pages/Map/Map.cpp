#include "Map.h"
#include <Arduino.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

Map::Map() : timer(nullptr), lastBtnTime(0), mode(MAP_MODE_PAN),
    centerLat(0), centerLon(0), zoom(0.01f), trackIdx(0),
    okPressStart(0), okLongHandled(false) {}

Map::~Map() {}

void Map::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Map::onViewLoad()
{
    Model.Init();
    View.Create(root);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);

    // Calculate initial view: fit entire track
    if (Model.GetTrackCount() > 1) {
        TrackPoint *pts = Model.GetTrackPoints();
        int count = Model.GetTrackCount();
        float minLat = pts[0].lat, maxLat = pts[0].lat;
        float minLon = pts[0].lon, maxLon = pts[0].lon;
        for (int i = 1; i < count; i++) {
            if (pts[i].lat < minLat) minLat = pts[i].lat;
            if (pts[i].lat > maxLat) maxLat = pts[i].lat;
            if (pts[i].lon < minLon) minLon = pts[i].lon;
            if (pts[i].lon > maxLon) maxLon = pts[i].lon;
        }
        centerLat = (minLat + maxLat) / 2.0f;
        centerLon = (minLon + maxLon) / 2.0f;
        // Zoom to fit: use larger of lat/lon range (with aspect correction)
        float latRange = (maxLat - minLat) * 1.2f; // 20% margin
        float lonRange = (maxLon - minLon) * 1.2f / 2.0f; // aspect ratio
        zoom = latRange > lonRange ? latRange : lonRange;
        if (zoom < 0.001f) zoom = 0.001f;
        trackIdx = 0;
    }

    mode = MAP_MODE_PAN;
    lastBtnTime = millis();
    okPressStart = 0;
    okLongHandled = false;

    timer = lv_timer_create(onTimer, 100, this);
}

void Map::onViewWillAppear() {
    // Hide StatusBar for fullscreen
    lv_obj_add_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Map::onViewDidAppear() {}
void Map::onViewWillDisappear() {
    if (timer) { lv_timer_del(timer); timer = nullptr; }
    // Restore StatusBar
    lv_obj_clear_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Map::onViewDidDisappear() {}
void Map::onViewDidUnload() {
    View.Delete();
    Model.Deinit();
}

void Map::onTimer(lv_timer_t *timer)
{
    Map *inst = (Map *)timer->user_data;
    inst->Model.Update();

    // Draw map
    inst->View.Draw(&inst->Model, inst->centerLat, inst->centerLon, inst->zoom, inst->trackIdx, inst->mode);

    uint32_t now = millis();
    bool ok = (digitalRead(BTN_OK) == LOW);
    bool up = (digitalRead(BTN_UP) == LOW);
    bool down = (digitalRead(BTN_DOWN) == LOW);

    // OK long press detection
    if (ok) {
        if (inst->okPressStart == 0) {
            inst->okPressStart = now;
            inst->okLongHandled = false;
        } else if (!inst->okLongHandled && (now - inst->okPressStart > 3000)) {
            // Long press: exit
            inst->Manager->Pop();
            inst->okLongHandled = true;
            return;
        }
    } else {
        if (inst->okPressStart != 0 && !inst->okLongHandled) {
            // Short press: toggle mode
            inst->mode = (inst->mode == MAP_MODE_PAN) ? MAP_MODE_ZOOM : MAP_MODE_PAN;
            Serial.printf("[Map] Mode: %s\n", inst->mode == MAP_MODE_PAN ? "PAN" : "ZOOM");
        }
        inst->okPressStart = 0;
    }

    // UP/DOWN with debounce
    if (now - inst->lastBtnTime < 150) return;

    if (up || down) {
        inst->lastBtnTime = now;

        if (inst->mode == MAP_MODE_PAN) {
            // Pan along track
            int count = inst->Model.GetTrackCount();
            if (count > 1) {
                int step = count / 20; // Move ~5% of track per press
                if (step < 1) step = 1;
                if (up) inst->trackIdx -= step;
                if (down) inst->trackIdx += step;
                if (inst->trackIdx < 0) inst->trackIdx = 0;
                if (inst->trackIdx >= count) inst->trackIdx = count - 1;

                // Move center to track point
                TrackPoint *pts = inst->Model.GetTrackPoints();
                inst->centerLat = pts[inst->trackIdx].lat;
                inst->centerLon = pts[inst->trackIdx].lon;
            }
        } else {
            // Zoom
            if (up) inst->zoom *= 0.7f;   // Zoom in
            if (down) inst->zoom *= 1.4f;  // Zoom out
            if (inst->zoom < 0.0005f) inst->zoom = 0.0005f;
            if (inst->zoom > 1.0f) inst->zoom = 1.0f;
        }
    }
}
