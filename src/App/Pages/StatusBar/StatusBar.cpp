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

    // 創建頂部容器（單色 OLED 標題區域）
    lv_obj_t* topBar = lv_obj_create(par);
    lv_obj_set_size(topBar, 128, 16); // 頂部狀態欄：電池 + 時間
    lv_obj_set_pos(topBar, 0, 0);

    // 設置背景樣式
    lv_obj_set_style_bg_color(topBar, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(topBar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(topBar, 0, 0);
    lv_obj_set_style_pad_all(topBar, 2, 0);

    // 創建標題標籤
    lv_obj_t* titleLabel = lv_label_create(topBar);
    lv_label_set_text(titleLabel, "StatusBar_Create");
    lv_obj_set_style_text_color(titleLabel, lv_color_white(), 0);
    lv_obj_center(titleLabel);

    // 添加狀態指示器（右側小點）
    lv_obj_t* statusDot = lv_obj_create(topBar);
    lv_obj_set_size(statusDot, 6, 6);
    lv_obj_set_pos(statusDot, 128 - 10, 5);
    lv_obj_set_style_bg_color(statusDot, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(statusDot, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(statusDot, 0, 0);
    lv_obj_set_style_radius(statusDot, 3, 0); // 圓形
    
    Serial.printf("OLED TopBar created: %s\n", "StatusBar_Create");
    return topBar;

}
