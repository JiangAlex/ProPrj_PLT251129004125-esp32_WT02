
#if 1
/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"
#include "App/Common/HAL/HAL.h"

static void keypad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) 
{
    
    // 1. 讀取按鍵的物理狀態 (低電平表示按下)
    bool ok_pressed = (digitalRead(BTN_MENU_OK_PIN) == LOW);
    bool up_pressed = (digitalRead(BTN_UP_BACK_PIN) == LOW);
    bool down_pressed = (digitalRead(BTN_DOWN_FN_PIN) == LOW);
    bool ptt_pressed = (digitalRead(BTN_PTT_PIN) == LOW);

    // 2. 確定哪個鍵被按下
    if (ok_pressed) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_ENTER; // OK 鍵映射為 LVGL 的 Enter 鍵
    } 
    else if (up_pressed) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_PREV;  // UP 鍵映射為 LVGL 的 Prev 鍵 (向上導航)
    }
    else if (down_pressed) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_NEXT;  // DOWN 鍵映射為 LVGL 的 Next 鍵 (向下導航)
    }
    else if (ptt_pressed) {
        data->state = LV_INDEV_STATE_PRESSED;
        data->key = LV_KEY_END;   // PTT 鍵映射為 END 鍵 (用於觸發 APRS TX/對講功能)
    }
    else {
        // 如果沒有按鍵被按下
        data->state = LV_INDEV_STATE_RELEASED;
        data->key = 0;
    }

    // 始終返回 false，表示不需要 LVGL 再次呼叫此函數
}

void lv_port_indev_init()
{
    // 1. 初始化底層 GPIO 
    HAL::Button_Init();
    // 2. 註冊輸入驅動
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    // 3. 設定類型和回調函數
    indev_drv.type = LV_INDEV_TYPE_KEYPAD; // 設定為鍵盤或鍵盤組
    indev_drv.read_cb = keypad_read;       // 註冊數據讀取函數
    //indev_drv.read_cb = HAL::Button_read;
    lv_indev_t * indev = lv_indev_drv_register(&indev_drv);
    // 5. 設定焦距組 (Group) 
    // 創建一個組，並將輸入設備與該組連接，用於在 UI 物件之間切換焦點
    static lv_group_t *group;
    group = lv_group_create();
    lv_indev_set_group(indev, group);
}
#endif