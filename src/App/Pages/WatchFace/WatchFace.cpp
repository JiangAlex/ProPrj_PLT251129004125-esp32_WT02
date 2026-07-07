#include "WatchFace.h"
#include "App/Common/HAL/HAL.h"
#include <WiFi.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

WatchFace::WatchFace() : timer(nullptr), lbl_time(nullptr), lbl_date(nullptr), lbl_status(nullptr), lastBtnTime(0) {}
WatchFace::~WatchFace() {}

void WatchFace::onCustomAttrConfig() {
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void WatchFace::onViewLoad() {
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 64);
    lv_obj_set_pos(root, 0, 0);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Time (large, center)
    lbl_time = lv_label_create(root);
    lv_obj_set_style_text_font(lbl_time, &lv_font_unscii_16, 0);
    lv_obj_set_style_text_color(lbl_time, lv_color_white(), 0);
    lv_obj_align(lbl_time, LV_ALIGN_CENTER, 0, -16);
    lv_label_set_text(lbl_time, "00:00");

    // Date (small, center)
    lbl_date = lv_label_create(root);
    lv_obj_set_style_text_font(lbl_date, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_date, lv_color_white(), 0);
    lv_obj_align(lbl_date, LV_ALIGN_CENTER, 0, 4);
    lv_label_set_text(lbl_date, "2026/01/01 MON");

    // Status (small, center-bottom)
    lbl_status = lv_label_create(root);
    lv_obj_set_style_text_font(lbl_status, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_status, lv_color_white(), 0);
    lv_obj_align(lbl_status, LV_ALIGN_CENTER, 0, 20);
    lv_label_set_text(lbl_status, "");

    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    lastBtnTime = millis();
    timer = lv_timer_create(onTimer, 500, this);
}

void WatchFace::onViewWillAppear() {
    lv_obj_add_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void WatchFace::onViewDidAppear() {}
void WatchFace::onViewWillDisappear() {
    if (timer) { lv_timer_del(timer); timer = nullptr; }
    lv_obj_clear_flag(lv_obj_get_child(lv_layer_top(), 0), LV_OBJ_FLAG_HIDDEN);
}
void WatchFace::onViewDidDisappear() {}
void WatchFace::onViewDidUnload() {
}

void WatchFace::onTimer(lv_timer_t *timer) {
    WatchFace *inst = (WatchFace *)timer->user_data;

    // Update time
    Clock_Info_t info;
    HAL::Clock_GetInfo(&info);

    char timeBuf[16];
    if (HAL::Clock_Is24Hour()) {
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", info.hour, info.minute);
    } else {
        int h = info.hour % 12;
        if (h == 0) h = 12;
        snprintf(timeBuf, sizeof(timeBuf), "%2d:%02d %s", h, info.minute, info.hour >= 12 ? "PM" : "AM");
    }
    lv_label_set_text(inst->lbl_time, timeBuf);

    // Update date
    static const char* weekShort[] = {"SUN","MON","TUE","WED","THU","FRI","SAT"};
    char dateBuf[20];
    snprintf(dateBuf, sizeof(dateBuf), "%04d/%02d/%02d %s", info.year, info.month, info.day, weekShort[info.week % 7]);
    lv_label_set_text(inst->lbl_date, dateBuf);

    // Update status
    GPS_Info_t gps;
    HAL::GPS_GetInfo(&gps);
    char statusBuf[24];

    if (HAL::PTT_IsPressed()) {
        snprintf(statusBuf, sizeof(statusBuf), "%s %s R  TX",
                 gps.isValid ? "G" : "g",
                 WiFi.status() == WL_CONNECTED ? "W" : "w");
    } else {
        int batt = HAL::Power_GetPercent();
        bool chg = HAL::Power_IsCharging();
        snprintf(statusBuf, sizeof(statusBuf), chg ? "%s %s R  %d%%+" : "%s %s R  %d%%",
                 gps.isValid ? "G" : "g",
                 WiFi.status() == WL_CONNECTED ? "W" : "w",
                 batt);
    }
    lv_label_set_text(inst->lbl_status, statusBuf);

    // Any key → back to StartUp
    uint32_t now = millis();
    if (now - inst->lastBtnTime < 500) return;

    if (digitalRead(BTN_UP) == LOW || digitalRead(BTN_DOWN) == LOW || digitalRead(BTN_OK) == LOW) {
        inst->Manager->Pop();
    }
}
