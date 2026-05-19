#include "HAL.h"

static bool ota_update_available = false;

void HAL::OTA_SetUpdateAvailable(bool available) {
    ota_update_available = available;
}

bool HAL::OTA_IsUpdateAvailable() {
    return ota_update_available;
}
