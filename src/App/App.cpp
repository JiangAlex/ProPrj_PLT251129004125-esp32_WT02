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
//extern lv_indev_t *indev_touchpad;

void App_Init()
{
    static AppFactory factory;
    static PageManager manager(&factory);
    /* Make sure the default group exists */
    if (!lv_group_get_default())
    {
        lv_group_t *group = lv_group_create();
        lv_group_set_default(group);
        //lv_indev_set_group(indev_touchpad, group);
    }

    /* Initialize the data processing node */
    // Initialize DataProc System after LVGL (requires lv_mem_alloc)
    Serial.println("Initializing DataProc System...");
    Serial.print("Free heap before DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    DataProc_Init();
    Serial.print("Free heap after DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println("DataProc System initialized");
    //ACCOUNT_SEND_CMD(Storage, STORAGE_CMD_LOAD);
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_LOAD);

    /* Set screen style */
    //Page::StatusBar_Create(lv_scr_act());
    /* Set root default style */

    /* Initialize resource pool */

    /* Initialize pages first */
    Serial.println("Installing Startup page...");
    manager.Install("Startup", "Pages/Startup");
    Serial.println("Setting animation type...");
    manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP,500);
    Serial.println("Pushing Startup page...");
    manager.Push("Pages/Startup");
    Serial.println("StartUp page push completed!");
    
    // 手动触发 LVGL 处理，确保页面加载事件被执行
    //delay(10);
    //lv_timer_handler();
    //delay(10);
    //lv_timer_handler();
    Serial.println("LVGL timer handler called manually");
    
    /* Initialize status bar - temporarily disabled */
    // Page::StatusBar_Create(lv_scr_act());
    
    
    #ifdef ENABLE_AUTO_OTA_CHECK
        App_Auto_OTA();
    #endif
}

void App_Uninit()
{
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
    //ACCOUNT_SEND_CMD(Recorder,  RECORDER_CMD_STOP);
}

void App_Auto_OTA ()
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
    // 嘗試自動連接到保存的 WiFi
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

    // Initialize OTA if WiFi is connected
    if (wifiManager.isConnected()) {
        Serial.println("=== OTA Initialization ===");  //VERSION_SOFTWARE
        // Initialize OTA updater
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