#ifndef SYSTEM_MODEL_H
#define SYSTEM_MODEL_H

#include "App/Common/DataProc/DataProc.h"
#include <Arduino.h>

namespace Page
{
    class SystemModel
    {
    public:
        void Init();
        void Deinit();
        void Update();

        // Getters
        bool IsWiFiConnected();
        String GetIP();
        uint32_t GetFreeHeap();
        uint8_t GetBrightness();
        uint8_t GetGPSSatellites();
        const char* GetVersion();

        // Actions
        void SetBrightness(uint8_t val);
        void ResetWiFi();
        void Reboot();
        void TriggerOTA();

    private:
        Account* account;
        uint8_t brightness;
    };
}

#endif
