#ifndef __RADIO_MODEL_H
#define __RADIO_MODEL_H

#include "App/Common/DataProc/DataProc.h"
#include "App/Common/HAL/HAL.h"

namespace Page
{

class RadioModel
{
public:
    void Init();
    void DeInit();
    void PlayMusic(const char* music);
    void SetEncoderEnable(bool en)
    {
//        HAL::Encoder_SetEnable(en);
    }
    void SetStatusBarAppear(bool en);
    
    // SA818 無線電控制
    int GetChannel();
    void SetChannel(int ch);
    int GetCTCSSIndex();
    void SetCTCSSIndex(int idx);
    bool IsHighPower();
    void SetHighPower(bool high);
    int GetVolume();
    void SetVolume(int vol);
    int GetSquelch();
    void SetSquelch(int sq);
    int GetRSSI();
    
    // 獲取頻率字符串
    float GetFrequency();

private:
    Account* account = nullptr;  // 初始化為 nullptr
    
    // 本地緩存的 SA818 狀態
    int cachedChannel = 1;
    int cachedCTCSSIndex = 0;  // 0 = OFF
    bool cachedHighPower = false;
    int cachedVolume = 4;
    int cachedSquelch = 4;
};

}

#endif
