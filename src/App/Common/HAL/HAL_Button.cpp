#include "HAL.h"
#include "App/Utils/PageManager/PageManager.h"
#include <Arduino.h>

static uint32_t buttonPressStartTime[3] = {0, 0, 0};  // Track when each button was pressed
static bool buttonLongPressTriggered[3] = {false, false, false};  // Prevent repeat triggers

void HAL::Button_Init()
{
    // 假設這些按鈕連接到 GND，並使用內部上拉
    pinMode(CONFIG_MENU_OK_PIN, INPUT_PULLUP);
    pinMode(CONFIG_UP_BACK_PIN, INPUT_PULLUP);
    pinMode(CONFIG_DOWN_FN_PIN, INPUT_PULLUP);
    pinMode(CONFIG_PTT_PIN, INPUT_PULLUP);
    
    // Initialize tracking arrays
    for (int i = 0; i < 3; i++) {
        buttonPressStartTime[i] = 0;
        buttonLongPressTriggered[i] = false;
    }
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
            pin = CONFIG_MENU_OK_PIN;
            break;
        case BUTTON_UP_BACK:
            pin = CONFIG_UP_BACK_PIN;
            break;
        case BUTTON_DOWN_FN:
            pin = CONFIG_DOWN_FN_PIN;
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
            pin = CONFIG_MENU_OK_PIN;
            break;
        case BUTTON_UP_BACK:
            pin = CONFIG_UP_BACK_PIN;
            break;
        case BUTTON_DOWN_FN:
            pin = CONFIG_DOWN_FN_PIN;
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

void HAL::Button_CheckLongPress()
{
    constexpr uint32_t LONG_PRESS_DURATION_MS = 3000;  // 3 seconds
    
    for (int i = 0; i < 3; i++) {
        bool isPressed = HAL::Button_IsPressed(i);
        
        if (isPressed && !buttonLongPressTriggered[i]) {
            // Button is pressed and hasn't triggered yet
            if (buttonPressStartTime[i] == 0) {
                // Start tracking press time
                buttonPressStartTime[i] = millis();
            } else {
                // Check if long press duration has been reached
                uint32_t elapsed = millis() - buttonPressStartTime[i];
                if (elapsed >= LONG_PRESS_DURATION_MS) {
                    // Long press detected - trigger navigation to Startup
                    buttonLongPressTriggered[i] = true;
                    
                    Serial.printf("Button %d: Long press detected, navigating to Startup\n", i);
                    
                    // Play beep confirmation
                    HAL::Buzzer_Beep(1000, 100);
                    
                    // Navigate to Startup page
                    PageManager::GetInstance()->Pop();
                    PageManager::GetInstance()->Push("Pages/Startup");
                }
            }
        } else if (!isPressed) {
            // Button released - reset tracking
            buttonPressStartTime[i] = 0;
            buttonLongPressTriggered[i] = false;
        }
    }
}
