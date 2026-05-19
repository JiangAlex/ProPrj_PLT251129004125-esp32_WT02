#include "SystemModel.h"
#include "App/Common/HAL/HAL.h"
#include "App/Configs/Version.h"
#include "App/Utils/OTA/ota_updater.h"
#include <WiFi.h>
#include <EEPROM.h>
#include <Preferences.h>

using namespace Page;

void SystemModel::Init()
{
    account = new Account("SystemModel", DataProc::Center(), 0, this);
    // Load brightness from NVS
    Preferences prefs;
    prefs.begin("system", true);
    brightness = prefs.getUChar("bright", 5);
    prefs.end();
    HAL::U8g2_SetBrightness(brightness * 32 - 1);
}

void SystemModel::Deinit()
{
    if (account) { delete account; account = nullptr; }
}

void SystemModel::Update() {}

bool SystemModel::IsWiFiConnected() { return WiFi.status() == WL_CONNECTED; }

String SystemModel::GetIP() {
    if (WiFi.status() == WL_CONNECTED) return WiFi.localIP().toString();
    return "N/A";
}

uint32_t SystemModel::GetFreeHeap() { return ESP.getFreeHeap(); }

uint8_t SystemModel::GetBrightness() { return brightness; }

uint8_t SystemModel::GetGPSSatellites() {
    GPS_Info_t info;
    HAL::GPS_GetInfo(&info);
    return info.satellites;
}

const char* SystemModel::GetVersion() { return VERSION_SOFTWARE; }

void SystemModel::SetBrightness(uint8_t val) {
    if (val < 1) val = 1;
    if (val > 8) val = 8;
    brightness = val;
    HAL::U8g2_SetBrightness(val * 32 - 1);
    Preferences prefs;
    prefs.begin("system", false);
    prefs.putUChar("bright", val);
    prefs.end();
}

int8_t SystemModel::GetTimezone() { return HAL::Clock_GetTimezone(); }

void SystemModel::SetTimezone(int8_t tz) {
    HAL::Clock_SetTimezone(tz);
    HAL::Clock_SyncNTP();
}

bool SystemModel::Is24Hour() { return HAL::Clock_Is24Hour(); }

void SystemModel::Toggle24Hour() {
    HAL::Clock_Set24Hour(!HAL::Clock_Is24Hour());
}

void SystemModel::ResetWiFi() {
    EEPROM.begin(512);
    EEPROM.put(128, (uint16_t)0x0000); // Clear magic number
    EEPROM.commit();
    delay(500);
    ESP.restart();
}

void SystemModel::Reboot() {
    ESP.restart();
}

void SystemModel::TriggerOTA() {
    Serial.println("[System] OTA update triggered");
    otaUpdater.performUpdate();
}

bool SystemModel::IsOTAAvailable() {
    return HAL::OTA_IsUpdateAvailable();
}
