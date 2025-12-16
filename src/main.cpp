#include <Arduino.h>
#include "App/Common/HAL/HAL.h"
#include "App/Common/Port/Display.h"
#include "App/App.h"

#ifdef USE_TEST
#include "test/Test.h"
#endif

#ifdef ENABLE_AUTO_OTA_CHECK
  #include "App/Utils/WiFiManager/wifi_manager.h"
  #include "App/Utils/OTA/ota_updater.h"
  static WiFiManager wifiManager; // WiFi Manager instance
#endif

void setup() {
  HAL::HAL_Init(); /* HAL Initialization */
  Port_Init(); /* Port Initialization */
  App_Init(); /* Application Initialization */
  #ifdef USE_TEST
    TEST::Test_Init();
  #endif
  //HAL::SA818_scan();
}

void loop() {
  #ifdef USE_TEST
    TEST::Test_App();
  #endif

  #ifdef ENABLE_AUTO_OTA_CHECK
    // Handle WiFi Manager portal
    wifiManager.loop();
  
    // Handle automatic OTA checks (only if WiFi is connected)
    if (wifiManager.isConnected()) {
      otaUpdater.handleAutoCheck();
    }
  #endif

  HAL::HAL_Update(); /* HAL Update */
  delay(5);  //5ms
}