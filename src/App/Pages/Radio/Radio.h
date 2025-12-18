#ifndef __RADIO_PRESENTER_H
#define __RADIO_PRESENTER_H

#include "RadioView.h"
#include "RadioModel.h"

namespace Page
{

// GPIO 按鍵定義
#define RADIO_BTN_UP    33
#define RADIO_BTN_DOWN  34
#define RADIO_BTN_OK    32

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
    
    // 按鍵處理
    void handleButtonUp();
    void handleButtonDown();
    void handleButtonOK();

private:
    RadioView View;
    RadioModel Model;
    
    // 定時器
    lv_timer_t* btnTimer = nullptr;
    
    // 按鍵防抖
    uint32_t lastBtnTime = 0;
    static const uint32_t DEBOUNCE_MS = 200;
    
    // 上次按鍵狀態
    bool lastBtnUp = true;
    bool lastBtnDown = true;
    bool lastBtnOK = true;
};

}

#endif
