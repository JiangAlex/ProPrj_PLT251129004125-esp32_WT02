#include "Display.h"
#include "../HAL/HAL.h"

TaskHandle_t handleTaskLvgl;
SemaphoreHandle_t xGuiSemaphore;

#if 0
void TaskLvglUpdate(void* parameter)
{
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    for (;;)
    {
        lv_task_handler();

        delay(5);
    }
}
#else
void TaskLvglUpdate(void *pvParameters) {
    // 這是 LVGL 任務，通常固定在 Core 1
    for (;;) {
        // 嘗試取得 Mutex，無限期等待 (portMAX_DELAY)
        if (xSemaphoreTake(xGuiSemaphore, portMAX_DELAY) == pdTRUE) {
            lv_timer_handler(); // 執行 LVGL 定時任務、重繪、動畫
            //處理完畢，務必釋放 Mutex
            xSemaphoreGive(xGuiSemaphore);
        }
            // 使用 vTaskDelay 進行延遲，以釋放 CPU 給其他任務 (例如 APRS TX)
            vTaskDelay(pdMS_TO_TICKS(5)); // 5ms 延遲，約 200Hz 刷新率
    }
}
#endif

void Port_Init()
{
	lv_init();
	lv_port_disp_init();
	lv_port_indev_init();

	// Update display in parallel thread.
    xGuiSemaphore = xSemaphoreCreateMutex();
    if (xGuiSemaphore == NULL) {
        log_e("建立 GUI Mutex 失敗！");
    }
    xTaskCreate(
        TaskLvglUpdate,
        "LvglThread",
        20000,
        nullptr,
        configMAX_PRIORITIES - 1,
        &handleTaskLvgl);
    
    /* set gradual */
    //HAL::Backlight_SetGradual(500, 1000);
}