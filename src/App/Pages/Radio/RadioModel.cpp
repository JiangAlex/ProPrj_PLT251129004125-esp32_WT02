#include "RadioModel.h"

using namespace Page;

void RadioModel::Init()
{
    account = new Account("RadioModel", DataProc::Center(), 0, this);
    //account->Subscribe("MusicPlayer");
    account->Subscribe("StatusBar");
    account->Subscribe("SA818");
    
    // 從 HAL 讀取當前 SA818 狀態
    cachedChannel = HAL::SA818_GetChannel();
    cachedHighPower = (HAL::SA818_GetPowerMode() == SA818_HIGH_POWER);
    cachedVolume = HAL::SA818_GetVolume();
    cachedSquelch = HAL::SA818_GetSquelch();
    cachedCTCSSIndex = HAL::SA818_GetCTCSS();
    
    Serial.printf("[RadioModel] Init: CH=%d, Power=%s, Vol=%d, CTCSS=%d, SQ=%d\n", 
                  cachedChannel, cachedHighPower ? "HIGH" : "LOW", 
                  cachedVolume, cachedCTCSSIndex, cachedSquelch);
}

void RadioModel::DeInit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
}

void RadioModel::PlayMusic(const char* music)
{
    if (!account) {
        Serial.println("[RadioModel] account is null, skipping PlayMusic");
        return;
    }
    DataProc::MusicPlayer_Info_t info;
    info.music = music;
    account->Notify("MusicPlayer", &info, sizeof(info));
}

void RadioModel::SetStatusBarAppear(bool en)
{
    if (!account) {
        Serial.println("[RadioModel] account is null, skipping SetStatusBarAppear");
        return;
    }
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.cmd = DataProc::STATUS_BAR_CMD_APPEAR;
    info.param.appear = en;
    account->Notify("StatusBar", &info, sizeof(info));
}

// ========== SA818 控制函數 ==========

int RadioModel::GetChannel()
{
    return cachedChannel;
}

void RadioModel::SetChannel(int ch)
{
    if (ch < 1) ch = 1;
    if (ch > 20) ch = 20;
    
    cachedChannel = ch;
    SA818_PowerMode mode = cachedHighPower ? SA818_HIGH_POWER : SA818_LOW_POWER;
    HAL::SA818_SetChannel(ch, mode);
    
    Serial.printf("[RadioModel] SetChannel: %d\n", ch);
}

int RadioModel::GetCTCSSIndex()
{
    return cachedCTCSSIndex;
}

void RadioModel::SetCTCSSIndex(int idx)
{
    // CTCSS 範圍 0-38 (0 = OFF)
    if (idx < 0) idx = 0;
    if (idx > 38) idx = 38;
    
    cachedCTCSSIndex = idx;
    HAL::SA818_SetCTCSS(idx);  // 實際設定到 SA818
    
    Serial.printf("[RadioModel] SetCTCSSIndex: %d\n", idx);
}

bool RadioModel::IsHighPower()
{
    return cachedHighPower;
}

void RadioModel::SetHighPower(bool high)
{
    cachedHighPower = high;
    HAL::SA818_SetHighLowPower(high);
    
    // 重新設定頻道以更新頻率
    SA818_PowerMode mode = high ? SA818_HIGH_POWER : SA818_LOW_POWER;
    HAL::SA818_SetChannel(cachedChannel, mode);
    
    Serial.printf("[RadioModel] SetHighPower: %s\n", high ? "HIGH" : "LOW");
}

int RadioModel::GetVolume()
{
    return cachedVolume;
}

void RadioModel::SetVolume(int vol)
{
    if (vol < 1) vol = 1;
    if (vol > 8) vol = 8;
    
    cachedVolume = vol;
    HAL::SA818_SetVolume(vol);
    
    Serial.printf("[RadioModel] SetVolume: %d\n", vol);
}

int RadioModel::GetSquelch()
{
    return cachedSquelch;
}

void RadioModel::SetSquelch(int sq)
{
    if (sq < 0) sq = 0;
    if (sq > 8) sq = 8;
    
    cachedSquelch = sq;
    HAL::SA818_SetSquelch(sq);  // 實際設定到 SA818
    
    Serial.printf("[RadioModel] SetSquelch: %d\n", sq);
}

int RadioModel::GetRSSI()
{
    // TODO: 從 SA818 讀取實際 RSSI
    // 目前返回模擬值
    return -85;
}

float RadioModel::GetFrequency()
{
    return HAL::SA818_GetCurrentFrequency();
}
