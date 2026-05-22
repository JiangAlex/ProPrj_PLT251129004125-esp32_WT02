#ifndef SYSTEM_H
#define SYSTEM_H

#include "App/Utils/PageManager/PageBase.h"
#include "App/Pages/Page.h"
#include "SystemView.h"
#include "SystemModel.h"

namespace Page
{
    class System : public PageBase
    {
    public:
        System();
        virtual ~System();

        virtual void onCustomAttrConfig() override;
        virtual void onViewLoad() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        void handleOK();
        SystemView View;
        SystemModel Model;
        lv_timer_t *timer;
        uint32_t lastBtnTime;
        uint32_t okPressStart;
        bool okLongHandled;
    };
}

#endif
