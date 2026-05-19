#include "App.h"
#include "App/Common/HAL/HAL.h"
#include "App/Common/DataProc/DataProc.h"
#include "App/Pages/AppFactory.h"
#include "App/Pages/StatusBar/StatusBar.h"
#include "App/Utils/PageManager/PageManager.h"

using namespace Page;

#ifdef ENABLE_AUTO_OTA_CHECK
  #include "App/Utils/WiFiManager/wifi_manager.h"
  #include "App/Utils/OTA/ota_updater.h"
  static WiFiManager wifiManager; // WiFi Manager instance
#endif
    
#define ACCOUNT_SEND_CMD(ACT, CMD)                                         \
    do                                                                     \
    {                                                                      \
        DataProc::ACT##_Info_t info;                                       \
        DATA_PROC_INIT_STRUCT(info);                                       \
        info.cmd = DataProc::CMD;                                          \
        DataProc::Center()->AccountMain.Notify(#ACT, &info, sizeof(info)); \
    } while (0)

void App_Init()
{
    static AppFactory factory;
    static PageManager manager(&factory);

    /* Initialize the data processing node */
    Serial.print("Free heap before DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    DataProc_Init();
    Serial.print("Free heap after DataProc init: ");
    Serial.println(ESP.getFreeHeap());

    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_LOAD);

    /* 設置螢幕背景為黑色（單色 OLED 必須） */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
    
    /* 設置 lv_layer_top 為透明 */
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);

    /* Initialize status bar - 放在 lv_layer_top 確保始終顯示在最上層 */
    Page::StatusBar_Create(lv_layer_top());

    /* Initialize pages first */
    manager.Install("Startup", "Pages/Startup");
    manager.Install("Radio",   "Pages/Radio");
    manager.Install("Trekking","Pages/Trekking");
    manager.Install("Map",     "Pages/Map");
    manager.Install("System", "Pages/System");
    manager.Install("Status",  "Pages/Status");
    manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP, 500);
    manager.Push("Pages/Startup");
    
    #ifdef ENABLE_AUTO_OTA_CHECK
        App_Auto_OTA();
    #endif
}

void App_Uninit()
{
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
}

static void App_Auto_OTA ()
{
    #ifdef OTA_SERVER_URL
        String serverURL = OTA_SERVER_URL;
    #else
        String serverURL = "http://your-server.com/firmware";
    #endif
 
    #ifdef OTA_VERSION_URL
        String versionURL = OTA_VERSION_URL;
    #else
        String versionURL = "http://your-server.com/version";
    #endif
 
    #ifdef OTA_CHECK_INTERVAL
        unsigned long interval = OTA_CHECK_INTERVAL;
    #else
        unsigned long interval = 3600; // 1 hour default
    #endif

    #ifdef CURRENT_VERSION
        String currentVersion = CURRENT_VERSION;
    #else
        String currentVersion = "1.0.0";
    #endif

    #ifdef ENABLE_AUTO_OTA_CHECK
    bool connected = wifiManager.autoConnectToWiFi();
    Serial.printf("WiFi connection result: %s\n", connected ? "Success" : "Failed");
    if (connected) {
      Serial.println("Connected to WiFi network!");
      Serial.print("IP address: ");
      Serial.println(wifiManager.getIP());
      Serial.printf("Connected to SSID: %s\n", wifiManager.getSSID().c_str());
    } else {
      Serial.println("Failed to connect to saved WiFi.");
      Serial.println("Starting WiFi configuration portal...");
      wifiManager.startConfigPortal();
      Serial.println("OTA functionality will be available after WiFi setup.");
    }
    Serial.println("===== WiFi Manager Debug End =====");

    if (wifiManager.isConnected()) {
        Serial.println("=== OTA Initialization ===");
        otaUpdater.begin(currentVersion, serverURL, versionURL, interval);
        otaUpdater.enableAutoCheck(true);

        Serial.println("Performing initial OTA check...");
        if (otaUpdater.checkForUpdates()) {
            Serial.println("Update available! Will update on next check cycle.");
        } else {
            Serial.println("No updates available.");
        }
    Serial.println("=== OTA Initialization Complete ===");
    } else {
        Serial.println("WiFi not connected - OTA disabled");
    }
    #endif
}
