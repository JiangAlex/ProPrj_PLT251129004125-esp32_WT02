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

    // 時間標籤 - 放在 topBar 內
    lv_obj_t* timeLabel = lv_label_create(topBar);
    if (!timeLabel) {
        Serial.println("ERROR: Failed to create timeLabel!");
        return topBar;
    }
    
    lv_label_set_text(timeLabel, "00:00");
    lv_obj_set_style_text_font(timeLabel, &lv_font_unscii_16, 0);  // 使用 8px 字體
    lv_obj_set_style_text_color(timeLabel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(timeLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(timeLabel, LV_ALIGN_LEFT_MID, 2, 0);  // 左側居中對齊
    
    // 右側電池標籤
    lv_obj_t* batteryLabel = lv_label_create(topBar);
    lv_label_set_text(batteryLabel, "100%");
    lv_obj_set_style_text_font(batteryLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(batteryLabel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(timeLabel, LV_OPA_TRANSP, 0);
    lv_obj_align(batteryLabel, LV_ALIGN_RIGHT_MID, -2, 0);  // 右側居中對齊
    
    Serial.printf("StatusBar created with 8px font\n");
    return topBar;

}
