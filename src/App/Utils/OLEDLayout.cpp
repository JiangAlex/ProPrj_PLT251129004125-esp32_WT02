#include "OLEDLayout.h"
#include <Arduino.h>
#include <lvgl.h>

lv_obj_t* OLEDLayout::createTopBar(lv_obj_t* parent, const char* title) {
    // 創建頂部容器（單色 OLED 標題區域）
    lv_obj_t* topBar = lv_obj_create(parent);
    lv_obj_set_size(topBar, OLED_WIDTH, OLED_TOP_YELLOW_HEIGHT);
    lv_obj_set_pos(topBar, 0, 0);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(topBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_pad_all(topBar, 2, 0);
    
    // 創建標題標籤
    lv_obj_t* titleLabel = lv_label_create(topBar);
    lv_label_set_text(titleLabel, title);
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_center(titleLabel);
    
    // 添加狀態指示器（右側小點）
    lv_obj_t* statusDot = lv_obj_create(topBar);
    lv_obj_set_size(statusDot, 6, 6);
    lv_obj_set_pos(statusDot, OLED_WIDTH - 10, 5);
    lv_obj_set_style_bg_color(statusDot, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(statusDot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(statusDot, 0, 0);
    lv_obj_set_style_radius(statusDot, 3, 0); // 圓形
    
    Serial.printf("OLED TopBar created: %s\n", title);
    return topBar;
}

lv_obj_t* OLEDLayout::createContentArea(lv_obj_t* parent) {
    // 創建內容區域（單色 OLED 內容區域）
    lv_obj_t* contentArea = lv_obj_create(parent);
    lv_obj_set_size(contentArea, OLED_WIDTH, OLED_BOTTOM_BLUE_HEIGHT);
    lv_obj_set_pos(contentArea, 0, OLED_TOP_YELLOW_HEIGHT);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(contentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 4, 0);
    
    // 禁用滾動條
    lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_OFF);
    
    Serial.println("OLED ContentArea created");
    return contentArea;
}

void OLEDLayout::updateTopBarTitle(lv_obj_t* topBar, const char* newTitle) {
    if (!topBar) return;
    
    // 查找標題標籤（通常是第一個子對象）
    lv_obj_t* titleLabel = lv_obj_get_child(topBar, 0);
    if (titleLabel && lv_obj_check_type(titleLabel, &lv_label_class)) {
        lv_label_set_text(titleLabel, newTitle);
        lv_obj_center(titleLabel);
        Serial.printf("TopBar title updated: %s\n", newTitle);
    }
}

void OLEDLayout::applyOLEDStyle(lv_obj_t* obj) {
    if (!obj) return;
    
    // OLED優化樣式：高對比度、小字體
    lv_obj_set_style_text_color(obj, lv_color_white(), 0);
    lv_obj_set_style_bg_color(obj, lv_color_black(), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_border_color(obj, lv_color_white(), 0);
}

lv_obj_t* OLEDLayout::createStatusBar(lv_obj_t* parent) {
    // 創建16px高的狀態欄
    lv_obj_t* statusBar = lv_obj_create(parent);
    lv_obj_set_size(statusBar, OLED_WIDTH, 16);
    lv_obj_set_pos(statusBar, 0, 0);
    
    // 設置狀態欄樣式
    lv_obj_set_style_bg_color(statusBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(statusBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(statusBar, 0, 0);
    lv_obj_set_style_pad_all(statusBar, 1, 0);
    
    // Create time label (left side) - show time on the LEFT per spec
    lv_obj_t* timeLabel = lv_label_create(statusBar);
    lv_label_set_text(timeLabel, "14:30");
    lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_set_style_text_opa(timeLabel, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(timeLabel, 0, 0);
    lv_obj_set_style_border_width(timeLabel, 0, 0);
    lv_obj_set_style_bg_opa(timeLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 2, 0);

    // Create battery/power label (right side) - show percentage only
    lv_obj_t* batteryLabel = lv_label_create(statusBar);
    lv_label_set_text(batteryLabel, "85%");
    lv_obj_set_style_text_font(batteryLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(batteryLabel, lv_color_white(), 0);
    lv_obj_set_style_text_opa(batteryLabel, LV_OPA_COVER, 0);
    lv_obj_set_style_outline_width(batteryLabel, 0, 0);
    lv_obj_set_style_border_width(batteryLabel, 0, 0);
    lv_obj_set_style_bg_opa(batteryLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, -2, 0);

    Serial.println("Status bar created with time (left) and battery (right)");
    return statusBar;
}

void OLEDLayout::updateStatusBar(lv_obj_t* statusBar, const char* batteryText, const char* timeText) {
    if (!statusBar) return;

    // Child 0 is timeLabel (left), Child 1 is batteryLabel (right)
    lv_obj_t* timeLabel = lv_obj_get_child(statusBar, 0);
    if (timeLabel && lv_obj_check_type(timeLabel, &lv_label_class)) {
        lv_label_set_text(timeLabel, timeText);
    }

    lv_obj_t* batteryLabel = lv_obj_get_child(statusBar, 1);
    if (batteryLabel && lv_obj_check_type(batteryLabel, &lv_label_class)) {
        lv_label_set_text(batteryLabel, batteryText);
    }
}

// 新的三區域佈局方法

// 創建中間主資訊區（38px）
lv_obj_t* OLEDLayout::createMainContentArea(lv_obj_t* parent) {
    lv_obj_t* contentArea = lv_obj_create(parent);
    lv_obj_set_size(contentArea, OLED_WIDTH, OLED_MAIN_CONTENT_HEIGHT);
    lv_obj_set_pos(contentArea, 0, OLED_MAIN_CONTENT_Y);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(contentArea, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(contentArea, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(contentArea, 0, 0);
    lv_obj_set_style_pad_all(contentArea, 2, 0);
    
    // 禁用滾動條
    lv_obj_set_scrollbar_mode(contentArea, LV_SCROLLBAR_MODE_OFF);
    
    Serial.println("OLED Main Content Area created (38px)");
    return contentArea;
}

// 創建底部功能提示區（10px）
lv_obj_t* OLEDLayout::createHintBar(lv_obj_t* parent, const char* hintText) {
    lv_obj_t* hintBar = lv_obj_create(parent);
    lv_obj_set_size(hintBar, OLED_WIDTH, OLED_HINT_BAR_HEIGHT);
    lv_obj_set_pos(hintBar, 0, OLED_HINT_BAR_Y);
    
    // 設置背景樣式
    lv_obj_set_style_bg_color(hintBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(hintBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hintBar, 0, 0);
    lv_obj_set_style_pad_all(hintBar, 1, 0);
    lv_obj_set_scrollbar_mode(hintBar, LV_SCROLLBAR_MODE_OFF);
    
    // 創建提示標籤
    lv_obj_t* hintLabel = lv_label_create(hintBar);
    lv_label_set_text(hintLabel, hintText ? hintText : "SAT 11/30");
    lv_obj_set_style_text_font(hintLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(hintLabel, lv_color_white(), 0);
    lv_obj_set_style_text_align(hintLabel, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(hintLabel);
    
    Serial.printf("OLED Hint Bar created: %s\n", hintText);
    return hintBar;
}

// 更新功能提示區
void OLEDLayout::updateHintBar(lv_obj_t* hintBar, const char* hintText) {
    if (!hintBar || !hintText) return;
    
    // 更新提示標籤（第一個子對象）
    lv_obj_t* hintLabel = lv_obj_get_child(hintBar, 0);
    if (hintLabel && lv_obj_check_type(hintLabel, &lv_label_class)) {
        lv_label_set_text(hintLabel, hintText);
    }
}