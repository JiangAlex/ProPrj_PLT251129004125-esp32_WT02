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
        int8_t GetTimezone();
        bool Is24Hour();
        uint8_t GetGPSSatellites();
        const char* GetVersion();

        // Actions
        void SetBrightness(uint8_t val);
        void SetTimezone(int8_t tz);
        void Toggle24Hour();
        void ResetWiFi();
        void Reboot();
        void TriggerOTA();
        bool IsOTAAvailable();

    private:
        Account* account;
        uint8_t brightness;
    };
}

#endif
