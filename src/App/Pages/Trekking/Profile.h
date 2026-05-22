#ifndef PROFILE_H
#define PROFILE_H

#include "App/Utils/PageManager/PageBase.h"
#include "App/Pages/Page.h"

namespace Page
{
    #define PROFILE_MAX_PTS 128

    enum ProfileMode { PROFILE_MODE_PAN = 0, PROFILE_MODE_ZOOM };

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

    public:
        static lv_color_t cbuf[];

    private:
        static void onTimer(lv_timer_t *timer);
        void drawProfile();
        void loadGPXProfile();

        lv_timer_t *timer;
        lv_obj_t *canvas;
        lv_obj_t *lbl_scale;

        ProfilePoint gpxPts[PROFILE_MAX_PTS];
        int gpxPtCount;
        float gpxMinEle, gpxMaxEle, gpxTotalDist;

        // View state
        ProfileMode mode;
        float viewDistStart, viewDistEnd;
        float viewEleMin, viewEleMax;

        // Input
        uint32_t lastBtnTime;
        uint32_t okPressStart;
        bool okLongHandled;
    };
}
#endif
