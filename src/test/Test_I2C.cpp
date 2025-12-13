#include "Test.h"
#include "Wire.h"
#include "App/Configs/Config.h"

void TEST::Test_I2C_Init(bool startScan) {
    Serial.println("Test_I2C_Init: Initializing I2C Test...");

    // 在這裡加入 I2C 測試的初始化程式碼
    Wire.begin(CONFIG_MCU_SDA_PIN, CONFIG_MCU_SCL_PIN);
    if(!startScan)
        return;
    Serial.println("Test_I2C_Init: I2C Test initialized.");
}

void TEST::Test_I2C_Loop() {
    // 在這裡加入需要在主迴圈中執行的 I2C 測試程式碼
    // 例如，讀取或寫入 I2C 裝置的資料，並檢查結果是否正確
    Serial.println("Test_I2C_Loop: Running I2C Test loop...");
}

void TEST::Test_I2C_Scan() {
    HAL::I2C_Init(true);
}