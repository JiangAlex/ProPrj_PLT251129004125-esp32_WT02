#include "Map.h"
#include <Arduino.h>

using namespace Page;

#define BTN_UP 33
#define BTN_DOWN 34
#define BTN_OK 32

Map::Map() : timer(nullptr), lastBtnTime(0) {}
Map::~Map() {}

void Map::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Map::onViewLoad()
{
    Model.Init();
    View.Create(root);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_OK, INPUT_PULLUP);
    timer = lv_timer_create(onTimer, 200, this);
}

void Map::onViewWillAppear() {}
void Map::onViewDidAppear() {}

void Map::onViewWillDisappear()
{
    if (timer) { lv_timer_del(timer); timer = nullptr; }
}

void Map::onViewDidDisappear() {}

void Map::onViewDidUnload()
{
    View.Delete();
    Model.Deinit();
}

void Map::onTimer(lv_timer_t *timer)
{
    Map *inst = (Map *)timer->user_data;
    inst->Model.Update();
    inst->View.UpdateView(&inst->Model);

    uint32_t now = millis();
    if (now - inst->lastBtnTime < 200) return;

    bool up = (digitalRead(BTN_UP) == LOW);
    bool down = (digitalRead(BTN_DOWN) == LOW);
    bool ok = (digitalRead(BTN_OK) == LOW);

    if (!up && !down && !ok) return;
    inst->lastBtnTime = now;

    if (inst->View.IsInFuncArea()) {
        if (up) {
            inst->View.ExitFuncArea();
        } else if (down) {
            int f = inst->View.GetFuncSelected();
            if (f < MAP_FUNC_COUNT - 1) inst->View.SetFuncSelected(f + 1);
        } else if (ok) {
            if (inst->View.GetFuncSelected() == MAP_FUNC_BACK) {
                inst->Manager->Pop();
            }
            // WPT: cycle to next waypoint
            if (inst->View.GetFuncSelected() == MAP_FUNC_WPT) {
                int idx = inst->Model.GetTargetWpt() + 1;
                inst->Model.SetTargetWpt(idx);
            }
        }
        return;
    }

    // In map area: UP/DOWN switch waypoints, OK → FuncBar
    if (up) {
        int idx = inst->Model.GetTargetWpt() - 1;
        inst->Model.SetTargetWpt(idx);
    } else if (down) {
        int idx = inst->Model.GetTargetWpt() + 1;
        inst->Model.SetTargetWpt(idx);
    } else if (ok) {
        inst->View.EnterFuncArea();
    }
}
