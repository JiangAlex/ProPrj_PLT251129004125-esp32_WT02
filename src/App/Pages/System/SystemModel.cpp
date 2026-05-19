#include "SystemModel.h"
#include "App/Common/HAL/HAL.h"
#include "App/Configs/Version.h"
#include <WiFi.h>
#include <EEPROM.h>

using namespace Page;

void SystemModel::Init()
{
    account = new Account("SystemModel", DataProc::Center(), 0, this);
    brightness = 5;
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
    HAL::U8g2_SetBrightness(val * 32 - 1); // 1-8 → 31-255
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
    Serial.println("[System] OTA check triggered");
    // OTA will be handled via WebGUI upload (Task 10)
}
