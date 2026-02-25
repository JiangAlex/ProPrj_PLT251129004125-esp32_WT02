#ifndef __RADIO_MODEL_H
#define __RADIO_MODEL_H

#include "App/Common/DataProc/DataProc.h"
#include "App/Common/HAL/HAL.h"

namespace Page
{

class RadioModel
{
public:
    RadioModel();
    void Init();
    void Deinit();
    
    void onEvent(Account* account, Account::EventParam_t* param);
    void Update(const SA818_Info_t* info);
    
    // Getters
    int GetChannel();
    int GetCTCSSIndex();
    bool IsHighPower();
    int GetRSSI();
    int GetVolume();
    int GetSquelch();
    float GetFrequency();
    bool IsTransmitting();
    void GetSA818Info(SA818_Info_t* info);

    // Setters
    void SetChannel(int ch);
    void SetCTCSSIndex(int index);
    void SetHighPower(bool high);
    void SetVolume(int vol);
    void SetSquelch(int cql);
    float GetFrequencyFor(int channel, bool highPower);
    
    void PlayMusic(const char* music);
    
    bool IsDirty();
    void ClearDirty();

private:
    Account* account;
    SA818_Info_t sa818_info;
    bool is_dirty;
};

}

#endif