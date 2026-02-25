#include "StartUp.h"

using namespace Page;

// GPIO 按鍵定義
#define BTN_UP    33
#define BTN_DOWN  34
#define BTN_OK    32

Startup::Startup()
{
}

Startup::~Startup()
{
}

void Startup::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Startup::onViewLoad()
{
    Model.Init();
    Serial.println("[ChappieCore] StartUpLoad!");
    View.Create(root);
    
    // 設置 GPIO 為輸入（帶上拉）
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    
    // 創建定時器檢查按鍵
    btnTimer = lv_timer_create(onTimer, 100, this);  // 每 100ms 檢查一次
}

void Startup::onViewDidLoad()
{
    Serial.println("[ChappieCore] StartUpOk!");
}

void Startup::onViewWillAppear()
{
    Model.PlayMusic("Startup");
    if (View.ui.anim_timeline != nullptr) {
        lv_anim_timeline_start(View.ui.anim_timeline);
    }
    Serial.println("[ChappieCore] StartUpAnimOk!");
}

void Startup::onViewDidAppear()
{
    Serial.println("[ChappieCore] StartUpAnimOut!");
}

void Startup::onViewWillDisappear()
{
    Serial.println("[ChappieCore] StartUpWillDisappear!");
    
    // 刪除定時器（必須在頁面消失前刪除）
    if (btnTimer) {
        lv_timer_del(btnTimer);
        btnTimer = nullptr;
        Serial.println("[ChappieCore] Timer deleted");
    }
}

void Startup::onViewDidDisappear()
{
    Serial.println("[ChappieCore] StartUpDidDisappear!");
}

void Startup::onViewUnload()
{
    // 注意: 此函數不會被 PageManager 調用
    // 清理工作已移到 onViewWillDisappear
    View.Delete();
    Serial.println("[ChappieCore] StartUpUnload!");
    Model.DeInit();
}

void Startup::onViewDidUnload()
{
    Serial.println("[ChappieCore] StartUpDidUnload!");
}

void Startup::onTimer(lv_timer_t *timer)
{
    Startup *instance = (Startup *)timer->user_data;
    
    // 使用獨立的狀態變數與計時器，避免按鍵互相干擾
    static bool lastUp = true;
    static bool lastDown = true;
    static bool lastOk = true;
    
    static unsigned long lastTimeUp = 0;
    static unsigned long lastTimeDown = 0;
    static unsigned long lastTimeOk = 0;
    
    bool curUp = digitalRead(BTN_UP);
    bool curDown = digitalRead(BTN_DOWN);
    bool curOk = digitalRead(BTN_OK);
    
    unsigned long now = millis();
    
    // UP 按鍵
    if (now - lastTimeUp > 200) {
        if (lastUp && !curUp) { // Falling edge
            int sel = instance->View.GetSelected();
            if (sel > 0) {
                instance->View.SetSelected(sel - 1);
                Serial.printf("[Menu] UP -> %d\n", sel - 1);
            }
            lastTimeUp = now;
        }
    }
    lastUp = curUp;
    
    // DOWN 按鍵
    // 注意: GPIO 34 沒有內部上拉電阻，若無外部上拉可能會浮動導致誤觸發或卡死
    if (now - lastTimeDown > 200) {
        if (lastDown && !curDown) { // Falling edge
            int sel = instance->View.GetSelected();
            if (sel < instance->View.GetMenuCount() - 1) {
                instance->View.SetSelected(sel + 1);
                Serial.printf("[Menu] DOWN -> %d\n", sel + 1);
            }
            lastTimeDown = now;
        }
    }
    lastDown = curDown;
    
    // OK 按鍵
    if (now - lastTimeOk > 200) {
        if (lastOk && !curOk) { // Falling edge
            int sel = instance->View.GetSelected();
            Serial.printf("[Menu] OK pressed, selected: %d\n", sel);
            
            // 根據選擇跳轉頁面
            switch (sel) {
                case 0:  // Trekking
                    Serial.println("[Menu] -> Trekking");
                    // instance->_Manager->Push("Pages/Trekking");
                    break;
                case 1:  // Radio
                    Serial.println("[Menu] -> Radio");
                    instance->Manager->Push("Pages/Radio");
                    break;
                case 2:  // System
                    Serial.println("[Menu] -> System");
                    // instance->_Manager->Push("Pages/System");
                    break;
                case 3:  // Status
                    Serial.println("[Menu] -> Status");
                    // instance->_Manager->Push("Pages/Status");
                    break;
            }
            lastTimeOk = now;
        }
    }
    lastOk = curOk;
}

void Startup::onEvent(lv_event_t *event)
{
    Startup *instance = (Startup *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t *obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            // instance->Manager->Pop();
        }
    }
}
