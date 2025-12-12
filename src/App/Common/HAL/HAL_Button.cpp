#include "HAL.h"

void HAL::Button_Init()
{
    // 假設這些按鈕連接到 GND，並使用內部上拉
    pinMode(BTN_MENU_OK_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_BACK_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_FN_PIN, INPUT_PULLUP);
    pinMode(BTN_PTT_PIN, INPUT_PULLUP);
}

void HAL::Button_Update()
{
    // 這個函數可以用來更新按鈕狀態，如果需要的話
}
::Button_Event_t HAL::Button_GetEvent(uint8_t buttonIndex)
{
    // 根據按鈕索引返回按鈕事件
    // 這裡僅作為示例，實際實現需要根據硬體設計
    return BUTTON_EVENT_NONE;
}
bool HAL::Button_IsPressed(uint8_t buttonIndex)
{
    uint8_t pin;
    switch (buttonIndex) {
        case BUTTON_MENU_OK:
            pin = BTN_MENU_OK_PIN;
            break;
        case BUTTON_UP_BACK:
            pin = BTN_UP_BACK_PIN;
            break;
        case BUTTON_DOWN_FN:
            pin = BTN_DOWN_FN_PIN;
            break;
        default:
            return false; // 無效按鈕索引
    }
    return digitalRead(pin) == LOW; // 按下時為 LOW
}
bool HAL::Button_IsHold(uint8_t buttonIndex)
{
    // 這個函數可以用來檢查按鈕是否被長按
    // 這裡僅作為示例，實際實現需要根據硬體設計
    return false;
}
void HAL::Button_GetInfo(uint8_t buttonIndex, ::Button_Info_t *info)
{
    if (!info) return;

    uint8_t pin;
    switch (buttonIndex) {
        case BUTTON_MENU_OK:
            pin = BTN_MENU_OK_PIN;
            break;
        case BUTTON_UP_BACK:
            pin = BTN_UP_BACK_PIN;
            break;
        case BUTTON_DOWN_FN:
            pin = BTN_DOWN_FN_PIN;
            break;
        default:
            return; // 無效按鈕索引
    }

    info->pin = pin;
    info->isPressed = (digitalRead(pin) == LOW);
    // 其他欄位可以根據需要進行填充
}
void HAL::Button_HandleEvents()
{
    // 這個函數可以用來處理按鈕事件
    // 這裡僅作為示例，實際實現需要根據硬體設計
}
