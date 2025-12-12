#ifndef __DATA_PROC_H
#define __DATA_PROC_H

#include "App/Utils/DataCenter/DataCenter.h"
#include "App/Common/HAL/HAL_Def.h"
#include "DataProc_Def.h"
#include "../HAL/HAL.h"

#define DATA_PROC_INIT_DEF(name)   void _DP_##name##_Init(Account* account)
#define DATA_PROC_INIT_STRUCT(sct) memset(&sct, 0, sizeof(sct))

void DataProc_Init();

namespace DataProc
{

DataCenter* Center();
uint32_t    GetTick();
uint32_t    GetTickElaps(uint32_t prevTick);
const char* MakeTimeString(uint64_t ms, char* buf, uint16_t len);

// SA818 音量設定介面
inline void SetSA818Volume(uint8_t vol) {
    HAL::SA818_SetVolume(vol);
}
inline uint8_t GetSA818Volume() {
    return HAL::SA818_GetVolume();
}
// SA818 頻道設定介面
inline void SetSA818Channel(int ch, SA818_PowerMode mode) {
    HAL::SA818_SetChannel(ch, mode);
}
// SA818 取得頻道資訊
inline void GetSA818ChannelInfo(SA818_ChannelInfo_t* info) {
    HAL::SA818_GetChannelInfo(info);
}
}

#endif
