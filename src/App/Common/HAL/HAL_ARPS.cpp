#include "HAL.h"

TaskHandle_t taskAPRSHandle;

void HAL::MicroAPRS_Init() {
	Serial.println("HAL: Initializing MicroAPRS...");
    xTaskCreatePinnedToCore(taskAPRS,   /* Function to implement the task */
                            "taskAPRS", /* Name of the task */
                            8192,       /* Stack size in words */
                            NULL,       /* Task input parameter */
                            2,          /* Priority of the task */
                            &taskAPRSHandle, /* Task handle. */
                            0); /* Core where the task should run */

    Serial.println("App_Init END");
}

void HAL::MicroAPRS_Update() {
	Serial.println("HAL: MicroAPRS update");
}

static void taskAPRS(void *pvParameters) {
    Serial.println("Task <APRS> started");
}
