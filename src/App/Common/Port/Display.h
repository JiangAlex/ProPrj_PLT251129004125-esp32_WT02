#ifndef __DISPLAY_H
#define __DISPLAY_H


#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "App/Configs/Config.h"
#include "lvgl.h"

void Port_Init();
void lv_port_disp_init();
void lv_port_indev_init();

extern TaskHandle_t handleTaskLvgl;
#endif