
#include <stdint.h>
#include "HAL.h"

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
    
    // Initialize core modules
    HAL::I2C_Init(true);
    HAL::U8g2_Init(); // Initialize U8g2 for display
    #ifdef ENABLE_AUTO_OTA_CHECK
        HAL::WiFi_Init();
    #endif
    // Display_Init();
    //HAL::Button_Init();
    HAL::Clock_Init();  // 初始化時鐘
    // Power_Init();  // TODO: Implement Power_Init()
    HAL::SA818_Init();  // Enable SA818/DRA818 for testing
    //HAL::PTT_Init();    // Initialize PTT (Push To Talk)

    // Optional modules
    // Buzz_init();
    // Audio_Init();
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
    //__IntervalExecute(HAL::WiFi_Update(), 100);
    //__IntervalExecute(HAL::GPS_Update(), 500);
    //__IntervalExecute(HAL::ENV_Update(), 100);
    
    // Legacy encoder support
    // Encoder_Update();
}