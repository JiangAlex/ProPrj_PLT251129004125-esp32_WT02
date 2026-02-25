#include "RadioModel.h"
#include "App/Common/HAL/HAL.h"
#include "App/Common/HAL/SA818_Channels.h"

namespace Page {

static RadioModel* model_instance = nullptr;

static int onEvent_wrapper(Account* account, Account::EventParam_t* param)
{
    if (model_instance) {
        model_instance->onEvent(account, param);
    }
    return 0;
}

RadioModel::RadioModel() {
    account = nullptr;
    is_dirty = true; // Start dirty to force initial view update
    memset(&sa818_info, 0, sizeof(sa818_info));
    model_instance = this;
}

void RadioModel::Init()
{
    // Get initial state from HAL
    HAL::SA818_GetInfo(&sa818_info);

    account = new Account("RadioModel", DataProc::Center(), 0, this);
    account->SetEventCallback(onEvent_wrapper);
    account->Subscribe("SA818");
}

void RadioModel::Deinit()
{
    if (account)
    {
        delete account;
        account = nullptr;
    }
    if (model_instance == this) {
        model_instance = nullptr;
    }
}

void RadioModel::onEvent(Account* account, Account::EventParam_t* param)
{
    if (param->event == Account::EVENT_NOTIFY) {
        Update((const SA818_Info_t*)param->data_p);
    }
}

void RadioModel::Update(const SA818_Info_t* info)
{
    memcpy(&sa818_info, info, sizeof(SA818_Info_t));
    is_dirty = true;
}

// Getters
int RadioModel::GetChannel() { return sa818_info.channel; }
int RadioModel::GetCTCSSIndex() { return sa818_info.ctcss_rx; }
bool RadioModel::IsHighPower() { return HAL::SA818_GetPowerMode() == SA818_HIGH_POWER; }
int RadioModel::GetRSSI() { return sa818_info.rssi; }
int RadioModel::GetVolume() { return sa818_info.volume; }
int RadioModel::GetSquelch() { return sa818_info.squelch; }
float RadioModel::GetFrequency() { return sa818_info.freq_rx; }
bool RadioModel::IsTransmitting() { return HAL::PTT_IsPressed(); }

void RadioModel::GetSA818Info(SA818_Info_t* info) {
    memcpy(info, &sa818_info, sizeof(SA818_Info_t));
}

// Setters
void RadioModel::SetChannel(int ch) { HAL::SA818_SetChannel(ch, HAL::SA818_GetPowerMode()); }
void RadioModel::SetCTCSSIndex(int index) { HAL::SA818_SetCTCSS(index); }
void RadioModel::SetHighPower(bool high) {
    SA818_PowerMode new_mode = high ? SA818_HIGH_POWER : SA818_LOW_POWER;
    HAL::SA818_SetChannel(GetChannel(), new_mode);
}
void RadioModel::SetVolume(int vol) { HAL::SA818_SetVolume(vol); }
void RadioModel::SetSquelch(int cql) { HAL::SA818_SetSquelch(cql); }

float RadioModel::GetFrequencyFor(int channel, bool highPower) {
    SA818_PowerMode mode = highPower ? SA818_HIGH_POWER : SA818_LOW_POWER;
    return getSA818Frequency(mode, channel);
}

void RadioModel::PlayMusic(const char* music) {
    HAL::Audio_PlayMusic(music);
}

bool RadioModel::IsDirty() {
    return is_dirty;
}

void RadioModel::ClearDirty() {
    is_dirty = false;
}

} // namespace Page