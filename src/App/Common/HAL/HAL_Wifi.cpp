#include "HAL.h"
#include "App/Utils/WiFiManager/wifi_manager.h"
#include "App/Utils/OTA/ota_updater.h"
#include "App/Utils/OTA/ota_config.h"
#include "App/Configs/Version.h"
#include <Preferences.h>

static WiFiManager wifiManager;

bool HAL::WiFi_IsEnabled() {
    Preferences prefs;
    prefs.begin("system", true);
    bool en = prefs.getBool("wifi", false); // Default OFF
    prefs.end();
    return en;
}

void HAL::WiFi_SetEnabled(bool en) {
    Preferences prefs;
    prefs.begin("system", false);
    prefs.putBool("wifi", en);
    prefs.end();
}

// WiFi 連接成功回調
static void onWiFiConnected() {
    Serial.println("[HAL_WiFi] WiFi connected, syncing NTP...");
    HAL::Clock_SyncNTP();
#ifdef ENABLE_WEB_GUI
    HAL::WebServer_Init();
#endif
    // OTA auto-check
    otaUpdater.begin(VERSION_SOFTWARE, OTA_SERVER_URL, OTA_VERSION_URL);
    if (otaUpdater.checkForUpdates()) {
        Serial.println("[OTA] New version available!");
        HAL::OTA_SetUpdateAvailable(true);
    }
}

void HAL::WiFi_Init() {
	Serial.println("Initializing WiFi...");
	
	// 設置連接成功回調
	wifiManager.onConnected = onWiFiConnected;
	
	// 初始化 WiFi Manager
  	wifiManager.begin();
}

void HAL::WiFi_Update(void) {
	wifiManager.loop();
}
void HAL::WiFi_APSetEnable(bool en) {
	Serial.println("AP Mode...");
}
void HAL::WiFi_STASetEnable(bool en) {
	Serial.println("STA mode...");
}
void HAL::WiFi_GetInfo(::WiFi_Info_t *info) {
	Serial.println("GetInfo WiFi...");
}