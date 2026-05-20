#include "Profile.h"
#include "TrekkingModel.h"
#include "App/Common/HAL/HAL.h"
#include <SPIFFS.h>
#include <math.h>

using namespace Page;

#define BTN_UP 33
#define BTN_OK 32
#define CANVAS_W 128
#define CANVAS_H 34
#define DEG_TO_RAD 0.017453292519943295

// Static canvas buffer
lv_color_t Profile::cbuf[CANVAS_W * CANVAS_H];

static double haversine(float lat1, float lon1, float lat2, float lon2) {
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;
    double a = sin(dLat/2)*sin(dLat/2) +
               cos(lat1*DEG_TO_RAD)*cos(lat2*DEG_TO_RAD)*sin(dLon/2)*sin(dLon/2);
    return 6371000.0 * 2.0 * atan2(sqrt(a), sqrt(1-a));
}

Profile::Profile() : timer(nullptr), canvas(nullptr), lbl_func(nullptr),
    gpxPtCount(0), gpxMinEle(0), gpxMaxEle(0), gpxTotalDist(0), lastBtnTime(0) {}

Profile::~Profile() {}

void Profile::onCustomAttrConfig() {
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Profile::onViewLoad() {
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);
    lv_obj_set_pos(root, 0, 16);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Canvas for chart
    canvas = lv_canvas_create(root);
    lv_canvas_set_buffer(canvas, cbuf, CANVAS_W, CANVAS_H, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);

    // FuncBar
    lv_obj_t *bar = lv_obj_create(root);
    lv_obj_remove_style_all(bar);
    lv_obj_set_size(bar, 128, 14);
    lv_obj_set_pos(bar, 0, 34);
    lbl_func = lv_label_create(bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "         [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);

    loadGPXProfile();
    drawProfile();

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    timer = lv_timer_create(onTimer, 100, this);
}

void Profile::onViewWillAppear() {}
void Profile::onViewDidAppear() {}
void Profile::onViewWillDisappear() {
    if (timer) { lv_timer_del(timer); timer = nullptr; }
}
void Profile::onViewDidDisappear() {}
void Profile::onViewDidUnload() {
    lv_obj_clean(root);
}

void Profile::loadGPXProfile() {
    gpxPtCount = 0;
    gpxMinEle = 99999; gpxMaxEle = -99999; gpxTotalDist = 0;

    String path = TrekkingModel::activeGPXPath;
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
            gpxPts[gpxPtCount].dist = cumDist / 1000.0f; // km
            gpxPts[gpxPtCount].ele = pt.ele;
            if (pt.ele < gpxMinEle) gpxMinEle = pt.ele;
            if (pt.ele > gpxMaxEle) gpxMaxEle = pt.ele;
            gpxPtCount++;
        }
        prevPt = pt;
    }
    gpxTotalDist = cumDist / 1000.0f;
    f.close();

    // Add margin to elevation range
    if (gpxMaxEle - gpxMinEle < 10) { gpxMinEle -= 5; gpxMaxEle += 5; }

    Serial.printf("[Profile] Loaded %d pts, dist=%.1fkm, ele=%.0f~%.0fm\n",
                  gpxPtCount, gpxTotalDist, gpxMinEle, gpxMaxEle);
}

void Profile::drawProfile() {
    // Clear canvas to black
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    if (gpxPtCount < 2) {
        // No data - just leave black canvas
        return;
    }

    float eleRange = gpxMaxEle - gpxMinEle;

    // Draw elevation polyline pixel by pixel
    for (int i = 1; i < gpxPtCount; i++) {
        int x0 = (int)((gpxPts[i-1].dist / gpxTotalDist) * (CANVAS_W - 1));
        int x1 = (int)((gpxPts[i].dist / gpxTotalDist) * (CANVAS_W - 1));
        int y0 = CANVAS_H - 1 - (int)(((gpxPts[i-1].ele - gpxMinEle) / eleRange) * (CANVAS_H - 1));
        int y1 = CANVAS_H - 1 - (int)(((gpxPts[i].ele - gpxMinEle) / eleRange) * (CANVAS_H - 1));

        // Bresenham line
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        while (true) {
            if (x0 >= 0 && x0 < CANVAS_W && y0 >= 0 && y0 < CANVAS_H)
                lv_canvas_set_px_color(canvas, x0, y0, lv_color_white());
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

void Profile::onTimer(lv_timer_t *timer) {
    Profile *inst = (Profile *)timer->user_data;
    uint32_t now = millis();
    if (now - inst->lastBtnTime < 300) return;

    // Wait for all buttons released first (ignore entry press)
    if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_OK) == LOW) {
        if (inst->lastBtnTime == 0) return; // Still holding from entry
        inst->Manager->Pop();
        inst->lastBtnTime = now;
    } else {
        if (inst->lastBtnTime == 0) inst->lastBtnTime = now; // Buttons released, start accepting input
    }
}
