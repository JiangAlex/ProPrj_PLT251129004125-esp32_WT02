#ifndef STATUS_H
#define STATUS_H

#include "App/Utils/PageManager/PageBase.h"
#include "StatusView.h"
#include "StatusModel.h"

namespace Page
{
    class Status : public PageBase
    {
    public:
        Status();
        virtual ~Status();
        
        virtual void onCustomAttrConfig() override;
        virtual void onViewLoad() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        StatusView View;
        StatusModel Model;
        lv_timer_t *timer;
        uint32_t lastBtnTime;
    };
}

#endif
