#ifndef PROFILE_H
#define PROFILE_H

#include "App/Utils/PageManager/PageBase.h"
#include "App/Pages/Page.h"

namespace Page
{
    #define PROFILE_MAX_PTS 128

    struct ProfilePoint { float dist; float ele; };

    class Profile : public PageBase
    {
    public:
        Profile();
        virtual ~Profile();
        virtual void onCustomAttrConfig() override;
        virtual void onViewLoad() override;
        virtual void onViewWillAppear() override;
        virtual void onViewDidAppear() override;
        virtual void onViewWillDisappear() override;
        virtual void onViewDidDisappear() override;
        virtual void onViewDidUnload() override;

    private:
        static void onTimer(lv_timer_t *timer);
        void drawProfile();
        void loadGPXProfile();

        lv_timer_t *timer;
        lv_obj_t *canvas;
        lv_obj_t *lbl_func;
        static lv_color_t cbuf[];

        ProfilePoint gpxPts[PROFILE_MAX_PTS];
        int gpxPtCount;
        float gpxMinEle, gpxMaxEle, gpxTotalDist;

        uint32_t lastBtnTime;
    };
}
#endif
