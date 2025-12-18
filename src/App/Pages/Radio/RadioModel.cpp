#include "RadioModel.h"

using namespace Page;

void RadioModel::Init()
{
    account = new Account("RadioModel", DataProc::Center(), 0, this);
    //account->Subscribe("MusicPlayer");
    account->Subscribe("StatusBar");
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
    DataProc::MusicPlayer_Info_t info;
    info.music = music;
    account->Notify("MusicPlayer", &info, sizeof(info));
}

void RadioModel::SetStatusBarAppear(bool en)
{
    DataProc::StatusBar_Info_t info;
    DATA_PROC_INIT_STRUCT(info);
    info.cmd = DataProc::STATUS_BAR_CMD_APPEAR;
    info.param.appear = en;
    account->Notify("StatusBar", &info, sizeof(info));
}
