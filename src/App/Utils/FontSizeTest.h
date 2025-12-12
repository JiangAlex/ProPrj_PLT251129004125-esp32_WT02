#pragma once
#include <lvgl.h>

// 字體大小測試工具 - 幫助比較不同字體大小在OLED上的實際效果
class FontSizeTest {
public:
    // 創建字體大小對比測試頁面
    static lv_obj_t* createTestScreen() {
        lv_obj_t* screen = lv_obj_create(nullptr);
        lv_obj_set_style_bg_color(screen, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);
        
        // 測試文字
        const char* testText = "Test Abc 123";
        
        // UNSCII 8px 點陣字體測試
        lv_obj_t* label8 = lv_label_create(screen);
        lv_label_set_text_fmt(label8, "U8: %s", testText);
        lv_obj_set_style_text_font(label8, &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(label8, lv_color_white(), 0);
        lv_obj_set_pos(label8, 5, 5);
        
        // UNSCII 16px 點陣字體測試
        lv_obj_t* label16 = lv_label_create(screen);
        lv_label_set_text_fmt(label16, "U16: %s", testText);
        lv_obj_set_style_text_font(label16, &lv_font_unscii_16, 0);
        lv_obj_set_style_text_color(label16, lv_color_white(), 0);
        lv_obj_set_pos(label16, 5, 20);
        
        // Montserrat 12px 對比測試
        lv_obj_t* labelM12 = lv_label_create(screen);
        lv_label_set_text_fmt(labelM12, "M12: %s", testText);
        lv_obj_set_style_text_font(labelM12, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_color(labelM12, lv_color_white(), 0);
        lv_obj_set_pos(labelM12, 5, 40);
        
        return screen;
    }
    
    // 應用不同DPI設置測試
    static void testDPIEffect(lv_obj_t* label) {
        // 注意：DPI在運行時無法更改，這只是測試函數
        // 實際需要在lv_conf.h中修改LV_DPI_DEF
        lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_opa(label, LV_OPA_COVER, 0);
    }
};