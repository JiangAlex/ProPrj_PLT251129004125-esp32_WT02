#ifndef __PAGE_H
#define __PAGE_H
#include "lvgl.h"
#include "App/Utils/PageManager/PageManager.h"

#ifdef ENABLE_WEB_GUI
#include <Arduino.h>
#include "App/Configs/Config.h"
extern volatile uint8_t g_virtual_key;
extern volatile bool g_virtual_pressed;

static inline int virtualDigitalRead(uint8_t pin) __attribute__((unused));
static inline int virtualDigitalRead(uint8_t pin) {
    if (g_virtual_pressed) {
        if (pin == CONFIG_MENU_OK_PIN && g_virtual_key == 1) return LOW;
        if (pin == CONFIG_UP_BACK_PIN && g_virtual_key == 2) return LOW;
        if (pin == CONFIG_DOWN_FN_PIN && g_virtual_key == 3) return LOW;
    }
    return digitalRead(pin);
}
#define digitalRead(pin) virtualDigitalRead(pin)
#endif

#endif
