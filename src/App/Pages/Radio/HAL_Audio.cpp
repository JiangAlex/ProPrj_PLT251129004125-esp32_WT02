#include "App/Common/HAL/HAL.h"
#include <Arduino.h>

void HAL::Audio_Init()
{
    Serial.println("HAL_Audio: Initializing...");
    // TODO: Add actual audio hardware initialization here, e.g., I2S, DAC.
    Serial.println("HAL_Audio: Done.");
}

bool HAL::Audio_PlayMusic(const char* name)
{
    Serial.printf("HAL_Audio: Playing music: %s\n", name);
    // TODO: Add actual music playback logic here (e.g., from SD card or flash).
    // For now, just return true to indicate success.
    return true;
}