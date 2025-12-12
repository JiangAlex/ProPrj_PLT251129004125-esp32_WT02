#ifndef OLED_LAYOUT_H
#define OLED_LAYOUT_H

#include <lvgl.h>

// OLED 128x64 三區域佈局常數
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// 三區域高度定義
#define OLED_STATUS_BAR_HEIGHT 16    // 頂部狀態欄：電池 + 時間
#define OLED_MAIN_CONTENT_HEIGHT 38  // 中間主資訊區：內容切換
#define OLED_HINT_BAR_HEIGHT 10      // 底部功能提示區：操作提示

// 位置定義
#define OLED_STATUS_BAR_Y 0
#define OLED_MAIN_CONTENT_Y 16
#define OLED_HINT_BAR_Y 54

// 保持向下兼容的宏定義
#define OLED_TOP_HEIGHT OLED_STATUS_BAR_HEIGHT
#define OLED_BOTTOM_HEIGHT OLED_MAIN_CONTENT_HEIGHT
#define OLED_TOP_YELLOW_HEIGHT OLED_STATUS_BAR_HEIGHT
#define OLED_BOTTOM_BLUE_HEIGHT OLED_MAIN_CONTENT_HEIGHT

class OLEDLayout {
public:
    // 三區域佈局創建方法
    
    // 1. 創建頂部狀態欄（16px）：電池電量 + 時間
    static lv_obj_t* createStatusBar(lv_obj_t* parent);
    
    // 2. 創建中間主資訊區（38px）：內容切換區域
    static lv_obj_t* createMainContentArea(lv_obj_t* parent);
    
    // 3. 創建底部功能提示區（10px）：操作提示
    static lv_obj_t* createHintBar(lv_obj_t* parent, const char* hintText);
    
    // 狀態欄更新方法
    static void updateStatusBar(lv_obj_t* statusBar, const char* batteryText, const char* timeText);
    
    // 功能提示區更新方法
    static void updateHintBar(lv_obj_t* hintBar, const char* hintText);
    
    // 向下兼容方法
    static lv_obj_t* createTopBar(lv_obj_t* parent, const char* title);
    static lv_obj_t* createContentArea(lv_obj_t* parent);
    static void updateTopBarTitle(lv_obj_t* topBar, const char* newTitle);
    static void applyOLEDStyle(lv_obj_t* obj);
};

#endif // OLED_LAYOUT_H