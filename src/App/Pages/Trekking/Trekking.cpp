#include "Trekking.h"
#include "App/Utils/PageManager/PageManager.h"
#include <Arduino.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

Trekking::Trekking() : timer(nullptr)
    , isEntryMode(true)
    , pressStartTime(0)
    , isLongPressHandled(false)
    , lastBtnTime(0)
{
}

Trekking::~Trekking()
{
}

void Trekking::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Trekking::onViewLoad()
{
    Model.Init();
    View.Create(root);

    isEntryMode = false;
    pressStartTime = 0;
    isLongPressHandled = false;
    lastBtnTime = 0;
    View.SetEntryMode(false);
    
    // 確保按鍵 GPIO 模式正確
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);

    // 創建定時器處理按鍵與 UI 更新
    timer = lv_timer_create(onTimer, 100, this);
}

void Trekking::onViewWillAppear()
{
    Serial.println("[Trekking] View Will Appear");
}

void Trekking::onViewDidAppear()
{
    Serial.println("[Trekking] View Did Appear");
}

void Trekking::onViewWillDisappear()
{
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }
}

void Trekking::onViewDidDisappear()
{
}

void Trekking::onViewDidUnload()
{
    View.Delete();
    Model.Deinit();
}

void Trekking::onTimer(lv_timer_t *timer)
{
    Trekking *instance = (Trekking *)timer->user_data;
    instance->Model.Update();
    instance->View.UpdateView(&instance->Model);

    // Button handling
    const uint32_t DEBOUNCE_MS = 150;
    uint32_t now = millis();

    bool isOkPressed = (digitalRead(BTN_OK) == LOW);

    // OK button logic (with long press)
    if (isOkPressed) {
        if (instance->pressStartTime == 0) { // Just pressed
            instance->pressStartTime = now;
            instance->isLongPressHandled = false;
        } else if (!instance->isLongPressHandled && (now - instance->pressStartTime > 3000)) {
            // Long press = return to menu
            instance->Manager->Pop();
            Serial.println("[Trekking] BACK (Long Press)");
            instance->isLongPressHandled = true; // Prevent short press action on release
        }
    } else { // Not pressed
        if (instance->pressStartTime != 0) { // Was pressed, now released
            if (!instance->isLongPressHandled) {
                // Short press OK
                if (!instance->View.IsInFuncArea()) {
                    int sel = instance->View.GetSelected();
                    if (sel == TrekkingView::TREKKING_ITEM_PROFILE) {
                        instance->Manager->Push("Pages/Profile");
                    } else if (sel == TrekkingView::TREKKING_ITEM_LIVE) {
                        instance->Manager->Push("Pages/Live");
                    } else if (sel == TrekkingView::TREKKING_ITEM_MAP) {
                        instance->Manager->Push("Pages/Map");
                    } else if (sel == TrekkingView::TREKKING_ITEM_GPX) {
                        int next = instance->Model.GetGPXSelected() + 1;
                        instance->Model.SetGPXSelected(next);
                    }
                } else {
                    int func = instance->View.GetFuncSelected();
                    if (func == TrekkingView::TREKKING_FUNC_START) {
                        if (instance->Model.IsRecording()) {
                            instance->Model.PauseRecord();
                            Serial.println("[Trekking] Action: PAUSE");
                        } else if (instance->Model.GetTimeMs() > 0) {
                            // Paused state: STOP (reset)
                            instance->Model.StopRecord();
                            Serial.println("[Trekking] Action: STOP");
                        } else {
                            instance->Model.StartRecord();
                            Serial.println("[Trekking] Action: START");
                        }
                    } else if (func == TrekkingView::TREKKING_FUNC_BACK) {
                        Serial.println("[Trekking] Action: BACK");
                        instance->Manager->Pop();
                    }
                }
            }
            instance->pressStartTime = 0;
        }
    }

    // UP/DOWN button logic (simple debounce)
    if (now - instance->lastBtnTime > DEBOUNCE_MS) {
        if (digitalRead(BTN_UP) == LOW) {
            if (instance->View.IsInFuncArea()) {
                instance->View.ExitFuncArea();
                instance->View.SetSelected(instance->View.GetItemCount() - 1);
            } else {
                int sel = instance->View.GetSelected();
                if (sel > 0) {
                    instance->View.SetSelected(sel - 1);
                }
            }
            instance->lastBtnTime = now;
        } else if (digitalRead(BTN_DOWN) == LOW) {
            if (instance->View.IsInFuncArea()) {
                int current = instance->View.GetFuncSelected();
                int next = (current == TrekkingView::TREKKING_FUNC_START) ? TrekkingView::TREKKING_FUNC_BACK : TrekkingView::TREKKING_FUNC_START;
                instance->View.SetFuncSelected(next);
            } else {
                int sel = instance->View.GetSelected();
                if (sel < instance->View.GetItemCount() - 1) {
                    instance->View.SetSelected(sel + 1);
                } else {
                    instance->View.EnterFuncArea();
                }
            }
            instance->lastBtnTime = now;
        }
    }
    
}