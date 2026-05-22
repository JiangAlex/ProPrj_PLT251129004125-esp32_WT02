
#include <stdint.h>
#include "HAL.h"
#include <SPIFFS.h>

/**
 * @brief Task to handle hal timer
 *
 * @param param
 */
static void task_hal_update_handler(void *param)
{
    while (1)
    {
        HAL::HAL_Update();
        delay(10);
    }
    vTaskDelete(NULL);
}

void HAL::HAL_Init(void)
{
    Serial.begin(115200);
    delay(100);
    Serial.println("HAL_Init: Starting hardware abstraction layer...");
    
    // Initialize SPIFFS
    if (SPIFFS.begin(true)) {
        Serial.printf("SPIFFS: Mounted, total=%u, used=%u\n", SPIFFS.totalBytes(), SPIFFS.usedBytes());
    } else {
        Serial.println("SPIFFS: Mount FAILED");
    }
    
    // Initialize core modules
    HAL::I2C_Init(true);
    HAL::U8g2_Init(); // Initialize U8g2 for display
    #ifdef ENABLE_AUTO_OTA_CHECK
        if (HAL::WiFi_IsEnabled()) HAL::WiFi_Init();
    #endif
    #ifdef ENABLE_WEB_GUI
        if (HAL::WiFi_IsEnabled()) HAL::WiFi_Init();
    #endif
    // Display_Init();
    //HAL::Button_Init();
    HAL::Clock_Init();  // 初始化時鐘
    HAL::Clock_SyncNTP();  // WiFi 已連線則同步 NTP
    // Power_Init();  // TODO: Implement Power_Init()
    HAL::SA818_Init();  // Enable SA818/DRA818 for testing
    HAL::PTT_Init();    // Initialize PTT (Push To Talk)
    HAL::GPS_Init();    // Initialize GPS module

    // Optional modules
    // Buzz_init(); // Still not implemented
    Audio_Init();
    // Encoder_Init();

    // Create HAL update task
    xTaskCreate(
        task_hal_update_handler,
        "Hal",
        1024 * 5,
        nullptr,
        5,
        nullptr);
    Serial.println("HAL_Init: Hardware abstraction layer initialized");
}

void HAL::HAL_Update(void)
{
    // Update core modules
    //__IntervalExecute(HAL::Button_Update(), 20);
    
    // Update optional modules with intervals
    //__IntervalExecute(HAL::IMU_Update(), 200);
    //__IntervalExecute(HAL::MAG_Update(), 50);
    __IntervalExecute(HAL::Power_Update(), 500); 
    __IntervalExecute(HAL::SA818_Update(), 1000); // Periodically get RSSI
    __IntervalExecute(HAL::PTT_Update(), 20);     // Check PTT button state
    __IntervalExecute(HAL::Button_CheckLongPress(), 100);  // Check for 3-second long press (100ms polling)
    __IntervalExecute(HAL::GPS_Update(), 100);    // Feed GPS NMEA data
    #ifdef ENABLE_WEB_GUI
    __IntervalExecute(HAL::WebServer_Update(), 100);  // Push framebuffer at ~10 FPS
    __IntervalExecute(HAL::WiFi_Update(), 50);        // Handle WiFiManager portal
    #endif
    //__IntervalExecute(HAL::GPS_Update(), 500);
    //__IntervalExecute(HAL::ENV_Update(), 100);
    
    // Legacy encoder support
    // Encoder_Update();
}