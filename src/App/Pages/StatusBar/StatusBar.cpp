/*
 * MIT License
 * Copyright (c) 2021 _VIFEXTech
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "StatusBar.h"
#include "App/Pages/Page.h"
#include "App/Common/DataProc/DataProc.h"
#include "App/Common/HAL/HAL.h"

struct
{
    lv_obj_t *cont;

    struct
    {
        lv_obj_t *img;
        lv_obj_t *label;
    } satellite;

    lv_obj_t *imgSD;

    lv_obj_t *labelClock;

    lv_obj_t *labelRec;

    struct
    {
        lv_obj_t *img;
        lv_obj_t *objUsage;
        lv_obj_t *label;
    } battery;
    
    lv_timer_t *updateTimer;
} ui;

// 定時器回調：更新時間和電池
static void StatusBar_UpdateTimer(lv_timer_t *timer)
{
    if (!ui.labelClock) return;
    
    // 從 HAL 獲取時間
    Clock_Info_t clockInfo;
    HAL::Clock_GetInfo(&clockInfo);
    
    // 格式化時間字串
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", clockInfo.hour, clockInfo.minute);
    lv_label_set_text(ui.labelClock, timeBuf);
    
    // 更新電池（模擬值）
    if (ui.battery.label) {
        int battPercent = 85 + (millis() / 60000) % 15;  // 模擬電池
        char battBuf[16];
        snprintf(battBuf, sizeof(battBuf), "%d%%", battPercent);
        lv_label_set_text(ui.battery.label, battBuf);
    }
}

lv_obj_t *Page::StatusBar_Create(lv_obj_t *par)
{
    Serial.println("Creating Status Bar...");
    
    // 检查父容器是否有效
    if (!par) {
        Serial.println("ERROR: par is NULL!");
        return nullptr;
    }
    Serial.printf("[StatusBar_Create] par size: %d x %d\n", lv_obj_get_width(par), lv_obj_get_height(par));
  
    // 創建頂部容器（單色 OLED 標題區域）
    lv_obj_t* topBar = lv_obj_create(par);
    if (!topBar) {
        Serial.println("ERROR: Failed to create topBar!");
        return nullptr;
    }
    
    // 移除預設樣式（重要！避免白色背景）
    lv_obj_remove_style_all(topBar);
    lv_obj_set_size(topBar, 128, 16);
    lv_obj_set_pos(topBar, 0, 0);

    // 設置黑色背景
    lv_obj_set_style_bg_color(topBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_pad_all(topBar, 0, 0);
    
    ui.cont = topBar;

    // 時間標籤 - 放在 topBar 內
    lv_obj_t* timeLabel = lv_label_create(topBar);
    if (!timeLabel) {
        Serial.println("ERROR: Failed to create timeLabel!");
        return topBar;
    }
    
    // 從 HAL 獲取初始時間
    Clock_Info_t clockInfo;
    HAL::Clock_GetInfo(&clockInfo);
    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", clockInfo.hour, clockInfo.minute);
    
    lv_label_set_text(timeLabel, timeBuf);
    lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_16, 0);
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(timeLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 2, 0);
    
    ui.labelClock = timeLabel;
    
    // 右側電池標籤
    lv_obj_t* batteryLabel = lv_label_create(topBar);
    lv_label_set_text(batteryLabel, "100%");
    lv_obj_set_style_text_font(batteryLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(batteryLabel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(batteryLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, -2, 0);
    
    ui.battery.label = batteryLabel;
    
    // 創建定時器每秒更新時間
    ui.updateTimer = lv_timer_create(StatusBar_UpdateTimer, 1000, nullptr);
    
    Serial.printf("StatusBar created with real clock (current: %s)\n", timeBuf);
    return topBar;
}
