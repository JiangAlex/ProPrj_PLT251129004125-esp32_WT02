#pragma once
#include <lvgl.h>

// 字體測試工具 - 幫助診斷OLED上字體渲染問題
class FontTest {
public:
    // 測試不同字體在單色OLED上的顯示效果
    static void testFonts(lv_obj_t* parent) {
        // 測試文字內容 - 包含英文字母、數字和符號
        const char* testText = "Test: ABC 123 !@#";
        
        // 創建測試標籤
        lv_obj_t* testLabel = lv_label_create(parent);
        lv_label_set_text(testLabel, testText);
        lv_obj_set_style_text_color(testLabel, lv_color_white(), 0);
        
        // 應用針對單色OLED優化的設定
        applyOLEDOptimization(testLabel);
        
        // 定位到螢幕中央
        lv_obj_align(testLabel, LV_ALIGN_CENTER, 0, 0);
    }
    
    // 應用專為SSD1306單色OLED優化的字體設定
    static void applyOLEDOptimization(lv_obj_t* label) {
        // 使用12px Montserrat字體 - 最適合128x64 OLED
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        
        // 設定完全不透明度
        lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
        
        // 零字母間距防止點陣斷開
        lv_obj_set_style_text_letter_space(label, 0, 0);
        
        // 最小行間距優化垂直空間使用
        lv_obj_set_style_text_line_space(label, 1, 0);
        
        // 移除所有陰影和邊框效果
        lv_obj_set_style_shadow_width(label, 0, 0);
        lv_obj_set_style_outline_width(label, 0, 0);
        lv_obj_set_style_border_width(label, 0, 0);
        
        // 設定抗鋸齒為關閉（單色顯示不需要）
        lv_obj_set_style_text_opa(label, LV_OPA_100, 0);
    }
    
    // 提供備用字體測試選項
    static void setAlternativeFont(lv_obj_t* label, int fontOption) {
        switch(fontOption) {
            case 0:
                lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
                break;
            case 1:
                lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
                break;
            case 2:
                // 如果可用，使用更適合點陣顯示的字體
                #if LV_FONT_UNSCII_8
                lv_obj_set_style_text_font(label, &lv_font_unscii_8, 0);
                #else
                lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
                #endif
                break;
            default:
                lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        }
    }
};