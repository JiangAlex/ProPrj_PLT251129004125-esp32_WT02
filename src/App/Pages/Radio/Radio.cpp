#include "Radio.h"

using namespace Page;

Radio::Radio()
{
}

Radio::~Radio()
{
}

void Radio::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Radio::onViewLoad()
{
    Model.Init();
    Serial.println("[ChappieCore] RadioLoad!");
    View.Create(root);
    
    // 創建定時器檢查按鍵
    lv_timer_t *timer = lv_timer_create(onTimer, 100, this);  // 每 100ms 檢查一次
}

void Radio::onViewDidLoad()
{
    Serial.println("[ChappieCore] RadioOk!");
}

void Radio::onViewWillAppear()
{
    Model.PlayMusic("Radio");
    if (View.ui.anim_timeline != nullptr) {
        lv_anim_timeline_start(View.ui.anim_timeline);
    }
    Serial.println("[ChappieCore] RadioAnimOk!");
}

void Radio::onViewDidAppear()
{
    Serial.println("[ChappieCore] RadioAnimOut!");
}

void Radio::onViewWillDisappear()
{
    Serial.println("[ChappieCore] RadioWillDisappear!");
}

void Radio::onViewDidDisappear()
{
    Serial.println("[ChappieCore] RadioDidDisappear!");
}

void Radio::onViewUnload()
{
    View.Delete();
    Serial.println("[ChappieCore] RadioUnload!");
    Model.DeInit();
}

void Radio::onViewDidUnload()
{
    Serial.println("[ChappieCore] RadioDidUnload!");
}

void Radio::onTimer(lv_timer_t *timer)
{
    Radio *instance = (Radio *)timer->user_data;
}

void Radio::onEvent(lv_event_t *event)
{
    Radio *instance = (Radio *)lv_event_get_user_data(event);
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
