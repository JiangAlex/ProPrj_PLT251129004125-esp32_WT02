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

    // Reset all state variables to their initial values for robustness
    isEntryMode = true;
    pressStartTime = 0;
    isLongPressHandled = false;
    lastBtnTime = 0;
    View.SetEntryMode(true);
    
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
    // UpdateView will be called regardless, which handles redraws
    if (!instance->isEntryMode) {
        instance->View.UpdateView(&instance->Model);
    }

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
            // Long press action
            if (!instance->isEntryMode) {
                Serial.println("[Trekking] BACK to Menu (Long Press)");
                instance->Manager->Pop();
            }
            instance->isLongPressHandled = true; // Prevent short press action on release
        }
    } else { // Not pressed
        if (instance->pressStartTime != 0) { // Was pressed, now released
            if (!instance->isLongPressHandled) {
                // Short press action
                if (instance->isEntryMode) {
                    instance->isEntryMode = false;
                    instance->View.SetEntryMode(false);
                    Serial.println("[Trekking] Entered Data View");
                } else {
                    if (instance->View.IsInFuncArea()) {
                        int func = instance->View.GetFuncSelected();
                        if (func == TrekkingView::TREKKING_FUNC_START) {
                            if (instance->Model.IsRecording()) {
                                instance->Model.PauseRecord();
                                Serial.println("[Trekking] Action: PAUSE");
                            } else {
                                instance->Model.StartRecord();
                                Serial.println("[Trekking] Action: RESUME/START");
                            }
                        } else if (func == TrekkingView::TREKKING_FUNC_BACK) {
                            Serial.println("[Trekking] Action: BACK");
                            instance->Manager->Pop();
                        }
                    }
                }
            }
            instance->pressStartTime = 0;
        }
    }

    // UP/DOWN button logic (simple debounce)
    if (now - instance->lastBtnTime > DEBOUNCE_MS) {
        if (!instance->isEntryMode) {
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
    
}