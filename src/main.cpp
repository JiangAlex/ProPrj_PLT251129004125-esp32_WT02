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

extern SemaphoreHandle_t xGuiSemaphore;

void setup() {
  HAL::HAL_Init(); /* HAL Initialization */
  Port_Init(); /* Port Initialization */
  
  /* Hold GUI mutex during App_Init to prevent LVGL task from rendering
     before screen styles and widgets are fully initialized */
  xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
  App_Init(); /* Application Initialization */
  xSemaphoreGive(xGuiSemaphore);
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

  delay(5);  //5ms
}