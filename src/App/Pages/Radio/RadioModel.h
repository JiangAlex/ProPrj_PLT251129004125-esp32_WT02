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


private:
    Account* account;
};

}

#endif
