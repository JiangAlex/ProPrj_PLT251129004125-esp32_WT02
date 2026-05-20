#include "Profile.h"
#include "TrekkingModel.h"
#include "App/Common/HAL/HAL.h"
#include <SPIFFS.h>
#include <math.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32
#define CW 128
#define CH 64
#define DEG_TO_RAD 0.017453292519943295

lv_color_t Profile::cbuf[CW * CH];

static double haversine(float lat1, float lon1, float lat2, float lon2) {
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;
    double a = sin(dLat/2)*sin(dLat/2) +
               cos(lat1*DEG_TO_RAD)*cos(lat2*DEG_TO_RAD)*sin(dLon/2)*sin(dLon/2);
    return 6371000.0 * 2.0 * atan2(sqrt(a), sqrt(1-a));
}

Profile::Profile() : timer(nullptr), canvas(nullptr),
    gpxPtCount(0), gpxMinEle(0), gpxMaxEle(0), gpxTotalDist(0),
    mode(PROFILE_MODE_PAN), viewDistStart(0), viewDistEnd(0),
    viewEleMin(0), viewEleMax(0),
    lastBtnTime(0), okPressStart(0), okLongHandled(false) {}

Profile::~Profile() {}

void Profile::onCustomAttrConfig() {
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Profile::onViewLoad() {
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 64);
    lv_obj_set_pos(root, 0, 0); // Fullscreen
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    canvas = lv_canvas_create(root);
    lv_canvas_set_buffer(canvas, cbuf, CW, CH, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);

    loadGPXProfile();

    // Initial view: show full profile
    viewDistStart = 0;
    viewDistEnd = gpxTotalDist;
    viewEleMin = gpxMinEle;
    viewEleMax = gpxMaxEle;
    mode = PROFILE_MODE_PAN;

    drawProfile();

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    lastBtnTime = millis();
    okPressStart = 0;
    okLongHandled = false;
    timer = lv_timer_create(onTimer, 100, this);
}

void Profile::onViewWillAppear() {
    // Hide StatusBar
    lv_obj_add_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Profile::onViewDidAppear() {}
void Profile::onViewWillDisappear() {
    if (timer) { lv_timer_del(timer); timer = nullptr; }
    // Restore StatusBar
    lv_obj_clear_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Profile::onViewDidDisappear() {}
void Profile::onViewDidUnload() {
    lv_obj_clean(root);
}

void Profile::loadGPXProfile() {
    gpxPtCount = 0;
    gpxMinEle = 99999; gpxMaxEle = -99999; gpxTotalDist = 0;

    String path = TrekkingModel::activeGPXPath;
    if (path.length() == 0) {
        // Fallback: first file
        File idx = SPIFFS.open("/gpx/index.txt", "r");
        if (idx) {
            String line = idx.readStringUntil('\n');
            line.trim();
            idx.close();
            if (line.length() > 0) {
                char buf[32];
                snprintf(buf, sizeof(buf), "/gpx/%03d.bin", line.toInt());
                path = String(buf);
            }
        }
    }
    if (path.length() == 0 || !SPIFFS.exists(path)) return;
    File f = SPIFFS.open(path, "r");
    if (!f) return;

    uint16_t tc, wc;
    f.read((uint8_t*)&tc, 2);
    f.read((uint8_t*)&wc, 2);

    struct { float lat; float lon; float ele; } pt, prevPt;
    float cumDist = 0;
    int step = tc > PROFILE_MAX_PTS ? tc / PROFILE_MAX_PTS : 1;

    for (int i = 0; i < tc && gpxPtCount < PROFILE_MAX_PTS; i++) {
        f.read((uint8_t*)&pt, sizeof(pt));
        if (i > 0) {
            cumDist += haversine(prevPt.lat, prevPt.lon, pt.lat, pt.lon);
        }
        if (i % step == 0) {
            gpxPts[gpxPtCount].dist = cumDist / 1000.0f;
            gpxPts[gpxPtCount].ele = pt.ele;
            if (pt.ele < gpxMinEle) gpxMinEle = pt.ele;
            if (pt.ele > gpxMaxEle) gpxMaxEle = pt.ele;
            gpxPtCount++;
        }
        prevPt = pt;
    }
    gpxTotalDist = cumDist / 1000.0f;
    f.close();

    if (gpxMaxEle - gpxMinEle < 10) { gpxMinEle -= 5; gpxMaxEle += 5; }
}

void Profile::drawProfile() {
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    if (gpxPtCount < 2) return;

    float distRange = viewDistEnd - viewDistStart;
    float eleRange = viewEleMax - viewEleMin;
    if (distRange <= 0 || eleRange <= 0) return;

    // Draw elevation polyline
    for (int i = 1; i < gpxPtCount; i++) {
        if (gpxPts[i].dist < viewDistStart && gpxPts[i-1].dist < viewDistStart) continue;
        if (gpxPts[i].dist > viewDistEnd && gpxPts[i-1].dist > viewDistEnd) continue;

        int x0 = (int)((gpxPts[i-1].dist - viewDistStart) / distRange * (CW - 1));
        int x1 = (int)((gpxPts[i].dist - viewDistStart) / distRange * (CW - 1));
        int y0 = (CH-1) - (int)((gpxPts[i-1].ele - viewEleMin) / eleRange * (CH - 1));
        int y1 = (CH-1) - (int)((gpxPts[i].ele - viewEleMin) / eleRange * (CH - 1));

        // Bresenham
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int steps = 0;
        while (steps++ < 300) {
            if (x0 >= 0 && x0 < CW && y0 >= 0 && y0 < CH)
                lv_canvas_set_px_color(canvas, x0, y0, lv_color_white());
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    // Draw GPS position marker (inverted triangle ▽)
    GPS_Info_t gps;
    HAL::GPS_GetInfo(&gps);
    if (gps.isValid && gpxPtCount > 0) {
        // Find nearest track point to GPS
        float minD = 999999;
        float posDist = 0;
        for (int i = 0; i < gpxPtCount; i++) {
            float dLat = gps.latitude - gpxPts[i].ele; // reuse ele field? No, need lat/lon
            // We don't have lat/lon in ProfilePoint, approximate by checking distance
            // Use a simpler approach: find nearest by comparing with stored track points from SPIFFS
            // For now, use GPS altitude to find closest elevation match on visible range
        }
        // Better approach: find closest point by re-reading track lat/lon
        // Since we only have dist/ele in gpxPts, find nearest by GPS→track distance
        // Read from SPIFFS again for lat/lon
        String path = TrekkingModel::activeGPXPath;
        if (path.length() > 0 && SPIFFS.exists(path)) {
            File f = SPIFFS.open(path, "r");
            if (f) {
                uint16_t tc, wc;
                f.read((uint8_t*)&tc, 2);
                f.read((uint8_t*)&wc, 2);
                struct { float lat; float lon; float ele; } pt;
                float cumDist = 0;
                float prevLat = 0, prevLon = 0;
                int step = tc > PROFILE_MAX_PTS ? tc / PROFILE_MAX_PTS : 1;
                int ptIdx = 0;

                for (int i = 0; i < tc; i++) {
                    f.read((uint8_t*)&pt, sizeof(pt));
                    if (i > 0) {
                        cumDist += haversine(prevLat, prevLon, pt.lat, pt.lon);
                    }
                    if (i % step == 0 && ptIdx < gpxPtCount) {
                        float dLat = gps.latitude - pt.lat;
                        float dLon = gps.longitude - pt.lon;
                        float d = dLat*dLat + dLon*dLon;
                        if (d < minD) {
                            minD = d;
                            posDist = cumDist / 1000.0f;
                        }
                        ptIdx++;
                    }
                    prevLat = pt.lat;
                    prevLon = pt.lon;
                }
                f.close();
            }
        }

        // Draw inverted triangle at posDist
        int px = (int)((posDist - viewDistStart) / distRange * (CW - 1));

        if (px < 0) px = 0;  // Clamp left
        if (px >= CW) px = CW - 1; // Clamp right

        // ▽ inverted triangle (top-down)
        if (px >= 2 && px < CW - 2) {
            lv_canvas_set_px_color(canvas, px-2, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, px-1, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, px, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, px+1, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, px+2, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, px-1, 1, lv_color_white());
            lv_canvas_set_px_color(canvas, px, 1, lv_color_white());
            lv_canvas_set_px_color(canvas, px+1, 1, lv_color_white());
            lv_canvas_set_px_color(canvas, px, 2, lv_color_white());
        } else if (px <= 2) {
            // Left edge triangle
            lv_canvas_set_px_color(canvas, 0, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, 1, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, 2, 0, lv_color_white());
            lv_canvas_set_px_color(canvas, 0, 1, lv_color_white());
            lv_canvas_set_px_color(canvas, 1, 1, lv_color_white());
            lv_canvas_set_px_color(canvas, 0, 2, lv_color_white());
        }
    }

    // Distance scale at bottom
    // Draw tick marks every N km (auto-scale)
    float tickInterval = 1.0f; // km
    if (distRange > 20) tickInterval = 5.0f;
    else if (distRange > 10) tickInterval = 2.0f;
    else if (distRange > 5) tickInterval = 1.0f;
    else if (distRange > 2) tickInterval = 0.5f;
    else tickInterval = 0.2f;

    float firstTick = ceil(viewDistStart / tickInterval) * tickInterval;
    for (float d = firstTick; d <= viewDistEnd; d += tickInterval) {
        int tx = (int)((d - viewDistStart) / distRange * (CW - 1));
        if (tx >= 0 && tx < CW) {
            // Tick mark (3px tall at bottom)
            for (int ty = CH - 3; ty < CH; ty++)
                lv_canvas_set_px_color(canvas, tx, ty, lv_color_white());
        }
    }
    // Draw start/end distance values (simple: leftmost and rightmost)
    // Left: viewDistStart
    int startKm = (int)viewDistStart;
    // Draw single digit at bottom-left (pixel approximation)
    // Just draw a dot at each tick for simplicity - the ticks themselves indicate scale

    // Mode indicator top-right
    int ox = CW - 5, oy = 1;
    if (mode == PROFILE_MODE_PAN) {
        for (int y = 0; y < 5; y++) lv_canvas_set_px_color(canvas, ox, oy+y, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+1, oy, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+2, oy, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+2, oy+1, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+1, oy+2, lv_color_white());
    } else {
        lv_canvas_set_px_color(canvas, ox, oy, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+1, oy, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+2, oy, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+2, oy+1, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+1, oy+2, lv_color_white());
        lv_canvas_set_px_color(canvas, ox, oy+3, lv_color_white());
        lv_canvas_set_px_color(canvas, ox, oy+4, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+1, oy+4, lv_color_white());
        lv_canvas_set_px_color(canvas, ox+2, oy+4, lv_color_white());
    }
}

void Profile::onTimer(lv_timer_t *timer) {
    Profile *inst = (Profile *)timer->user_data;
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
            inst->Manager->Pop();
            inst->okLongHandled = true;
            return;
        }
    } else {
        if (inst->okPressStart != 0 && !inst->okLongHandled) {
            // Short press: toggle mode
            inst->mode = (inst->mode == PROFILE_MODE_PAN) ? PROFILE_MODE_ZOOM : PROFILE_MODE_PAN;
            inst->drawProfile();
        }
        inst->okPressStart = 0;
    }

    // UP/DOWN with debounce
    if (now - inst->lastBtnTime < 150) return;

    if (up || down) {
        inst->lastBtnTime = now;
        float distRange = inst->viewDistEnd - inst->viewDistStart;

        if (inst->mode == PROFILE_MODE_PAN) {
            float shift = distRange * 0.2f;
            if (up) { inst->viewDistStart -= shift; inst->viewDistEnd -= shift; }
            if (down) { inst->viewDistStart += shift; inst->viewDistEnd += shift; }
            if (inst->viewDistStart < 0) {
                inst->viewDistEnd -= inst->viewDistStart;
                inst->viewDistStart = 0;
            }
            if (inst->viewDistEnd > inst->gpxTotalDist) {
                inst->viewDistStart -= (inst->viewDistEnd - inst->gpxTotalDist);
                inst->viewDistEnd = inst->gpxTotalDist;
            }
        } else {
            float center = (inst->viewDistStart + inst->viewDistEnd) / 2.0f;
            if (up) distRange *= 0.7f;
            if (down) distRange *= 1.4f;
            if (distRange < 0.1f) distRange = 0.1f;
            if (distRange > inst->gpxTotalDist) distRange = inst->gpxTotalDist;
            inst->viewDistStart = center - distRange / 2.0f;
            inst->viewDistEnd = center + distRange / 2.0f;
            if (inst->viewDistStart < 0) { inst->viewDistStart = 0; inst->viewDistEnd = distRange; }
            if (inst->viewDistEnd > inst->gpxTotalDist) { inst->viewDistEnd = inst->gpxTotalDist; inst->viewDistStart = inst->gpxTotalDist - distRange; }
        }

        inst->drawProfile();
    }
}
