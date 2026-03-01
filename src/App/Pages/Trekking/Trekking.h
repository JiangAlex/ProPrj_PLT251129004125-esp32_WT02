#ifndef TREKKING_H
#define TREKKING_H

#include "App/Utils/PageManager/PageBase.h"
#include "TrekkingView.h"
#include "TrekkingModel.h"

namespace Page
{
    class Trekking : public PageBase
    {
    public:
        Trekking();
        virtual ~Trekking();

        virtual void onViewLoad() override;
        virtual void onCustomAttrConfig() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        TrekkingView View;
        TrekkingModel Model;
        lv_timer_t *timer;
        bool isEntryMode;
        uint32_t pressStartTime;
        bool isLongPressHandled;
        uint32_t lastBtnTime;
    };
}

#endif