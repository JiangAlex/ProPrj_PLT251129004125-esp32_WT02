#ifndef LIVE_H
#define LIVE_H

#include "App/Utils/PageManager/PageBase.h"
#include "App/Pages/Page.h"

namespace Page
{
    enum LiveMode { LIVE_MODE_PAN = 0, LIVE_MODE_ZOOM };

    class Live : public PageBase
    {
    public:
        Live();
        virtual ~Live();
        virtual void onCustomAttrConfig() override;
        virtual void onViewLoad() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        void drawLive();

        lv_timer_t *timer;
        lv_obj_t *canvas;
        lv_obj_t *lbl_scale;

        LiveMode mode;
        float viewTimeStart, viewTimeEnd;
        uint32_t lastBtnTime;
        uint32_t okPressStart;
        bool okLongHandled;
    };
}
#endif
