#ifndef __TEST_H
#define __TEST_H

#include <Arduino.h>
#include "lvgl.h"
//#include "../../lib/APRS-ESP/lib/LibAPRS_ESP32/LibAPRSesp.h"
#include "App/Common/HAL/HAL.h"
#include "../../lib/LibAPRS_ESP32/LibAPRSesp.h"
#include <ESP_Music.h>

namespace TEST
{   
    // Test 初始化函式
    void Test_Init();
    void Test_App();
    // MicroAPRS 測試相關函式
    void MicroAPRS_Init();
    void MicroAPRS_Loop(); // MicroAPRS 測試主迴圈函式
    // 音樂測試相關函式
    void Music_Init(); 
    void Music_Loop();  // 音樂測試主迴圈函式
    // UI 測試初始化函式
    void ui_test_init();
    // I2C 測試相關函式
    void Test_I2C_Init(bool startScan);
    void Test_I2C_Loop();
    void Test_I2C_Scan();
}
#endif