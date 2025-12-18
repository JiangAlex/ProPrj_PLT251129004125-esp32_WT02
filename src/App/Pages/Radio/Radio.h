#ifndef __RADIO_PRESENTER_H
#define __RADIO_PRESENTER_H

#include "RadioView.h"
#include "RadioModel.h"

namespace Page
{

class Radio : public PageBase
{
public:

public:
    Radio();
    virtual ~Radio();

    virtual void onCustomAttrConfig();
    virtual void onViewLoad();
    virtual void onViewDidLoad();
    virtual void onViewWillAppear();
    virtual void onViewDidAppear();
    virtual void onViewWillDisappear();
    virtual void onViewDidDisappear();
    virtual void onViewUnload();
    virtual void onViewDidUnload();

private:
    static void onTimer(lv_timer_t* timer);
    static void onEvent(lv_event_t* event);

private:
    RadioView View;
    RadioModel Model;
};

}

#endif
