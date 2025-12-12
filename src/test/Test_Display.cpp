#include "Test.h"

void TEST::ui_test_init() {
    // 創建一個新的螢幕
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_scr_load(scr); // 載入螢幕

    // 創建一個標籤
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, "APRS Tracker READY!");

    // 設定標籤位置 (置中)
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // 創建一個進度條 (Progress Bar) 進行視覺測試
    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 100, 10);
    lv_obj_align_to(bar, label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_bar_set_range(bar, 0, 100);
    lv_bar_set_value(bar, 75, LV_ANIM_ON); // 設置一個值，看它是否正確顯示
}