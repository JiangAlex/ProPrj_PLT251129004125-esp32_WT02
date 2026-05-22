#include "Live.h"
#include "TrekkingModel.h"
#include "App/Common/HAL/HAL.h"
#include <math.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32
#define CW 128
#define CH 64

// Reuse Profile's canvas buffer (they never display simultaneously)
#include "Profile.h"
#define Live_cbuf (Profile::cbuf)

Live::Live() : timer(nullptr), canvas(nullptr), lbl_scale(nullptr),
    mode(LIVE_MODE_PAN), viewTimeStart(0), viewTimeEnd(0),
    lastBtnTime(0), okPressStart(0), okLongHandled(false) {}

Live::~Live() {}

void Live::onCustomAttrConfig() {
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Live::onViewLoad() {
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 64);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    canvas = lv_canvas_create(root);
    lv_canvas_set_buffer(canvas, Live_cbuf, CW, CH, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(canvas, 0, 0);

    lbl_scale = lv_label_create(root);
    lv_obj_set_style_text_font(lbl_scale, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_scale, lv_color_white(), 0);
    lv_obj_set_style_bg_color(lbl_scale, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lbl_scale, LV_OPA_COVER, 0);
    lv_obj_align(lbl_scale, LV_ALIGN_BOTTOM_LEFT, 1, -1);
    lv_label_set_text(lbl_scale, "");

    // Initial view: fit all live data
    viewTimeStart = 0;
    viewTimeEnd = TrekkingModel::liveMaxTime > 0 ? TrekkingModel::liveMaxTime : 60;
    mode = LIVE_MODE_PAN;

    drawLive();

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    lastBtnTime = millis();
    okPressStart = 0;
    okLongHandled = false;
    timer = lv_timer_create(onTimer, 200, this);
}

void Live::onViewWillAppear() {
    lv_obj_add_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Live::onViewDidAppear() {}
void Live::onViewWillDisappear() {
    if (timer) { lv_timer_del(timer); timer = nullptr; }
    lv_obj_clear_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void Live::onViewDidDisappear() {}
void Live::onViewDidUnload() {}

void Live::drawLive() {
    lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);

    int count = TrekkingModel::livePtCount;
    if (count < 2) return;

    float timeRange = viewTimeEnd - viewTimeStart;
    if (timeRange <= 0) return;

    // Find min/max altitude in view
    float minEle = 99999, maxEle = -99999;
    for (int i = 0; i < count; i++) {
        float t = TrekkingModel::livePts[i].time_sec;
        if (t >= viewTimeStart && t <= viewTimeEnd) {
            float a = TrekkingModel::livePts[i].alt;
            if (a < minEle) minEle = a;
            if (a > maxEle) maxEle = a;
        }
    }
    if (minEle > maxEle) return;
    float eleRange = maxEle - minEle;
    if (eleRange < 10) { minEle -= 5; maxEle += 5; eleRange = maxEle - minEle; }

    // Draw elevation line
    for (int i = 1; i < count; i++) {
        float t0 = TrekkingModel::livePts[i-1].time_sec;
        float t1 = TrekkingModel::livePts[i].time_sec;
        if (t1 < viewTimeStart || t0 > viewTimeEnd) continue;

        int x0 = (int)((t0 - viewTimeStart) / timeRange * (CW - 1));
        int x1 = (int)((t1 - viewTimeStart) / timeRange * (CW - 1));
        int y0 = (CH-1) - (int)((TrekkingModel::livePts[i-1].alt - minEle) / eleRange * (CH - 1));
        int y1 = (CH-1) - (int)((TrekkingModel::livePts[i].alt - minEle) / eleRange * (CH - 1));

        // Bresenham
        int dx = abs(x1-x0), sx = x0<x1?1:-1;
        int dy = -abs(y1-y0), sy = y0<y1?1:-1;
        int err = dx+dy;
        int steps = 0;
        while (steps++ < 300) {
            if (x0>=0 && x0<CW && y0>=0 && y0<CH)
                lv_canvas_set_px_color(canvas, x0, y0, lv_color_white());
            if (x0==x1 && y0==y1) break;
            int e2=2*err;
            if (e2>=dy){err+=dy;x0+=sx;}
            if (e2<=dx){err+=dx;y0+=sy;}
        }
    }

    // Time scale ticks at bottom
    float tickInterval = 60; // seconds
    if (timeRange > 3600) tickInterval = 600;
    else if (timeRange > 1800) tickInterval = 300;
    else if (timeRange > 600) tickInterval = 120;
    else if (timeRange > 300) tickInterval = 60;
    else tickInterval = 30;

    float firstTick = ceil(viewTimeStart / tickInterval) * tickInterval;
    for (float t = firstTick; t <= viewTimeEnd; t += tickInterval) {
        int tx = (int)((t - viewTimeStart) / timeRange * (CW - 1));
        if (tx >= 0 && tx < CW) {
            for (int ty = CH-3; ty < CH; ty++)
                lv_canvas_set_px_color(canvas, tx, ty, lv_color_white());
        }
    }

    // Scale label
    if (lbl_scale) {
        char buf[12];
        if (tickInterval >= 60)
            snprintf(buf, sizeof(buf), "%dm", (int)(tickInterval / 60));
        else
            snprintf(buf, sizeof(buf), "%ds", (int)tickInterval);
        lv_label_set_text(lbl_scale, buf);
    }

    // Mode indicator top-right
    int ox = CW-5, oy = 1;
    if (mode == LIVE_MODE_PAN) {
        for (int y=0;y<5;y++) lv_canvas_set_px_color(canvas, ox, oy+y, lv_color_white());
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

void Live::onTimer(lv_timer_t *timer) {
    Live *inst = (Live *)timer->user_data;
    uint32_t now = millis();

    // Auto-update view end to follow latest data
    if (TrekkingModel::liveMaxTime > inst->viewTimeEnd) {
        inst->viewTimeEnd = TrekkingModel::liveMaxTime;
    }

    // Redraw periodically
    inst->drawLive();

    bool ok = (digitalRead(BTN_OK) == LOW);
    bool up = (digitalRead(BTN_UP) == LOW);
    bool down = (digitalRead(BTN_DOWN) == LOW);

    // OK long press
    if (ok) {
        if (inst->okPressStart == 0) { inst->okPressStart = now; inst->okLongHandled = false; }
        if (!inst->okLongHandled && (now - inst->okPressStart > 3000)) {
            inst->Manager->Pop();
            inst->okLongHandled = true;
            return;
        }
    } else {
        if (inst->okPressStart > 0 && !inst->okLongHandled) {
            inst->mode = (inst->mode == LIVE_MODE_PAN) ? LIVE_MODE_ZOOM : LIVE_MODE_PAN;
        }
        inst->okPressStart = 0;
    }

    // UP/DOWN
    if (now - inst->lastBtnTime < 150) return;
    if (up || down) {
        inst->lastBtnTime = now;
        float timeRange = inst->viewTimeEnd - inst->viewTimeStart;

        if (inst->mode == LIVE_MODE_PAN) {
            float shift = timeRange * 0.2f;
            if (up) { inst->viewTimeStart -= shift; inst->viewTimeEnd -= shift; }
            if (down) { inst->viewTimeStart += shift; inst->viewTimeEnd += shift; }
            if (inst->viewTimeStart < 0) { inst->viewTimeEnd -= inst->viewTimeStart; inst->viewTimeStart = 0; }
        } else {
            float center = (inst->viewTimeStart + inst->viewTimeEnd) / 2.0f;
            if (up) timeRange *= 0.7f;
            if (down) timeRange *= 1.4f;
            if (timeRange < 30) timeRange = 30;
            inst->viewTimeStart = center - timeRange / 2.0f;
            inst->viewTimeEnd = center + timeRange / 2.0f;
            if (inst->viewTimeStart < 0) { inst->viewTimeStart = 0; inst->viewTimeEnd = timeRange; }
        }
    }
}
