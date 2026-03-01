#include "System.h"
#include <Arduino.h>
#include "App/Utils/PageManager/PageManager.h"

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

System::System() : timer(nullptr), lastBtnTime(0)
{
}

System::~System()
{
}

void System::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void System::onViewLoad()
{
    Model.Init();
    Serial.println("[System] onViewLoad");
    View.Create(root);
    
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);

    timer = lv_timer_create(onTimer, 100, this);
}

void System::onViewWillAppear() {}
void System::onViewDidAppear() {}

void System::onViewWillDisappear()
{
    Serial.println("[System] onViewWillDisappear");
    if (timer) {
        lv_timer_del(timer);
        timer = nullptr;
    }
}

void System::onViewDidDisappear() {}

void System::onViewDidUnload()
{
    Serial.println("[System] onViewDidUnload");
    View.Delete();
    Model.Deinit();
}

void System::onTimer(lv_timer_t *timer)
{
    System *instance = (System *)timer->user_data;
    instance->Model.Update();
    instance->View.UpdateView(&instance->Model);

    uint32_t now = millis();
    if (now - instance->lastBtnTime > 150) {
        if (digitalRead(BTN_UP) == LOW) {
            instance->View.SetSelected(instance->View.GetSelected() - 1);
            instance->lastBtnTime = now;
        } else if (digitalRead(BTN_DOWN) == LOW) {
            instance->View.SetSelected(instance->View.GetSelected() + 1);
            instance->lastBtnTime = now;
        } else if (digitalRead(BTN_OK) == LOW) {
            // Simple Back implementation for now: Long press or just press
            // For consistency with other pages, let's use a simple press to select (placeholder)
            // and maybe long press to go back? Or just assume one of the items is "Back"
            // For now, let's make OK button just pop back to menu to allow navigation testing
            instance->Manager->Pop();
            instance->lastBtnTime = now + 500;
        }
    }
}
