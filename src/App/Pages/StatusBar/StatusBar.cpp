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
} ui;

lv_obj_t *Page::StatusBar_Create(lv_obj_t *par)
{
    Serial.println("Creating Status Bar...");
    
    // 检查父容器是否有效
    if (!par) {
        Serial.println("ERROR: par is NULL!");
        return nullptr;
    }

    //確保父容器（通常是畫面）是黑底且不透明，避免繼承默認白底導致整條亮起
    lv_obj_set_style_bg_color(par, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(par, LV_OPA_COVER, 0);

    // 創建頂部容器（單色 OLED 標題區域）
    lv_obj_t* topBar = lv_obj_create(par);
    if (!topBar) {
        Serial.println("ERROR: Failed to create topBar!");
        return nullptr;
    }
    
    // 移除預設樣式，避免默認背景導致整條亮起
    lv_obj_remove_style_all(topBar); // 移除所有預設樣式
    lv_obj_set_size(topBar, 128, 16); // 頂部狀態欄：電池 + 時間
    lv_obj_set_pos(topBar, 0, 0); // 靠頂部對齊

    // 設置背景樣式（全亮白底）
    lv_obj_set_style_bg_color(topBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_pad_all(topBar, 0, 0); // 移除內邊距

    // 暫時使用 LVGL label 但簡化配置
    lv_obj_t* timeLabel = lv_label_create(par);
    if (!timeLabel) {
        Serial.println("ERROR: Failed to create timeLabel!");
        return topBar;
    }
    
    lv_label_set_text(timeLabel, "00:00");
    lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_16, 0); // 使用16號字體
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_set_pos(timeLabel, 2, 0);
    
    Serial.printf("TimeLabel created with font 16\n");
#if 0
    // 右側電池標籤
    lv_obj_t* batteryLabel = lv_label_create(topBar);
    lv_label_set_text(batteryLabel, "Bat");
    lv_obj_set_style_text_color(batteryLabel, lv_color_black(), 0);
    lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, 0, 0);

    // 底部中央文字
    lv_obj_t* centerLabel = lv_label_create(par);
    lv_label_set_text(centerLabel, "SoftSnail");
    lv_obj_set_style_text_color(centerLabel, lv_color_white(), 0);
    lv_obj_align(centerLabel, LV_ALIGN_BOTTOM_MID, 0, -8);
#endif
    Serial.printf("OLED TopBar created: %s\n", "StatusBar_Create");
    return topBar;

}
