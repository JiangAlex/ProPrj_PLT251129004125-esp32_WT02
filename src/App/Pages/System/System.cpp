#include "System.h"
#include <Arduino.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

enum SysItem {
    SYS_WIFI = 0, SYS_IP, SYS_HEAP, SYS_BRIGHT, SYS_TIMEZONE,
    SYS_CLOCK_FMT, SYS_SCRTO, SYS_GPS, SYS_VERSION, SYS_OTA, SYS_RESET_WIFI, SYS_REBOOT
};

System::System() : timer(nullptr), lastBtnTime(0), okPressStart(0), okLongHandled(false) {}
System::~System() {}

void System::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void System::onViewLoad()
{
    Model.Init();
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
    if (timer) { lv_timer_del(timer); timer = nullptr; }
}

void System::onViewDidDisappear() {}

void System::onViewDidUnload()
{
    View.Delete();
    Model.Deinit();
}

void System::handleOK()
{
    if (View.IsInFuncArea()) {
        Manager->Pop();
        return;
    }

    int sel = View.GetSelected();

    if (View.IsEditMode()) {
        // Exit edit mode
        View.SetEditMode(false);
        return;
    }

    switch (sel) {
        case SYS_WIFI:
            Model.ToggleWiFi();
            break;
        case SYS_BRIGHT:
        case SYS_TIMEZONE:
        case SYS_SCRTO:
            View.SetEditMode(true);
            break;
        case SYS_CLOCK_FMT:
            Model.Toggle24Hour();
            break;
        case SYS_OTA:
            Model.TriggerOTA();
            break;
        case SYS_RESET_WIFI:
            Model.ResetWiFi();
            break;
        case SYS_REBOOT:
            Model.Reboot();
            break;
        default:
            break; // Read-only items
    }
}

void System::onTimer(lv_timer_t *timer)
{
    System *inst = (System *)timer->user_data;
    inst->View.UpdateView(&inst->Model);

    uint32_t now = millis();

    // Long press OK = return to menu
    bool okState = (digitalRead(BTN_OK) == LOW);
    bool okShortPress = false;
    if (okState) {
        if (inst->okPressStart == 0) inst->okPressStart = now;
        if (!inst->okLongHandled && (now - inst->okPressStart > 3000)) {
            inst->Manager->Pop();
            inst->okLongHandled = true;
            return;
        }
    } else {
        if (inst->okPressStart > 0 && !inst->okLongHandled) {
            okShortPress = true;
        }
        inst->okPressStart = 0;
        inst->okLongHandled = false;
    }

    if (now - inst->lastBtnTime < 150) return;

    bool up = (digitalRead(BTN_UP) == LOW);
    bool down = (digitalRead(BTN_DOWN) == LOW);
    bool ok = okShortPress;

    if (!up && !down && !ok) return;
    inst->lastBtnTime = now;

    if (inst->View.IsEditMode()) {
        int sel = inst->View.GetSelected();
        if (up) {
            if (sel == SYS_BRIGHT) {
                uint8_t b = inst->Model.GetBrightness();
                if (b < 8) inst->Model.SetBrightness(b + 1);
            } else if (sel == SYS_TIMEZONE) {
                int8_t tz = inst->Model.GetTimezone();
                if (tz < 14) inst->Model.SetTimezone(tz + 1);
            } else if (sel == SYS_SCRTO) {
                uint8_t t = HAL::Clock_GetScreenTimeout();
                if (t < 60) HAL::Clock_SetScreenTimeout(t + 5);
            }
        } else if (down) {
            if (sel == SYS_BRIGHT) {
                uint8_t b = inst->Model.GetBrightness();
                if (b > 1) inst->Model.SetBrightness(b - 1);
            } else if (sel == SYS_TIMEZONE) {
                int8_t tz = inst->Model.GetTimezone();
                if (tz > -12) inst->Model.SetTimezone(tz - 1);
            } else if (sel == SYS_SCRTO) {
                uint8_t t = HAL::Clock_GetScreenTimeout();
                if (t > 5) HAL::Clock_SetScreenTimeout(t - 5);
            }
        } else if (ok) {
            inst->View.SetEditMode(false);
        }
        return;
    }

    if (inst->View.IsInFuncArea()) {
        if (up) {
            inst->View.ExitFuncArea();
            inst->View.SetSelected(SYS_ITEM_COUNT - 1);
        } else if (down) {
            int f = inst->View.GetFuncSelected();
            if (f < SYS_FUNC_COUNT - 1) inst->View.SetFuncSelected(f + 1);
        } else if (ok) {
            inst->handleOK();
        }
        return;
    }

    // Normal navigation
    if (up) {
        int sel = inst->View.GetSelected();
        if (sel > 0) inst->View.SetSelected(sel - 1);
    } else if (down) {
        int sel = inst->View.GetSelected();
        if (sel < SYS_ITEM_COUNT - 1) {
            inst->View.SetSelected(sel + 1);
        } else {
            inst->View.EnterFuncArea();
        }
    } else if (ok) {
        inst->handleOK();
    }
}
