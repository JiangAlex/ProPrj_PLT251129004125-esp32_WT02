#include "HAL.h"
#include "App/Utils/WiFiManager/wifi_manager.h"

static WiFiManager wifiManager; // WiFi Manager instance

// WiFi 連接成功回調
static void onWiFiConnected() {
    Serial.println("[HAL_WiFi] WiFi connected, syncing NTP...");
    HAL::Clock_SyncNTP();
}

void HAL::WiFi_Init() {
	Serial.println("Initializing WiFi...");
	
	// 設置連接成功回調
	wifiManager.onConnected = onWiFiConnected;
	
	// 初始化 WiFi Manager
  	wifiManager.begin();
}

void HAL::WiFi_Update(void) {
	Serial.println("Update WiFi...");
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