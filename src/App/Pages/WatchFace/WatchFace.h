#ifndef WATCHFACE_H
#define WATCHFACE_H

#include "App/Utils/PageManager/PageBase.h"
#include "App/Pages/Page.h"

namespace Page
{
    class WatchFace : public PageBase
    {
    public:
        WatchFace();
        virtual ~WatchFace();
        virtual void onCustomAttrConfig() override;
        virtual void onViewLoad() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        lv_timer_t *timer;
        lv_obj_t *lbl_time;
        lv_obj_t *lbl_date;
        lv_obj_t *lbl_status;
        uint32_t lastBtnTime;
    };
}
#endif
