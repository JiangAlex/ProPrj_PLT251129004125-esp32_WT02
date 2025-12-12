#include <Arduino.h>
#include "App/Common/HAL/HAL.h"
#include "App/Common/Port/Display.h"
#include "App/App.h"
#ifdef USE_TEST
#include "test/Test.h"
#endif


void setup() {
  HAL::HAL_Init(); /* HAL Initialization */
  Port_Init(); /* Port Initialization */
  App_Init(); /* Application Initialization */
   
  //HAL::SA818_scan();
}

void loop() {
  #ifdef USE_TEST
    TEST::Test_Init();
  #endif
  HAL::HAL_Update(); /* HAL Update */
  delay(5);  //5ms
}