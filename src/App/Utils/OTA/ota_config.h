#ifndef OTA_CONFIG_H
#define OTA_CONFIG_H

// OTA Configuration Settings
// These can be overridden by build flags in platformio.ini

#ifndef CURRENT_VERSION
#define CURRENT_VERSION "1.0.0"
#endif

#ifndef OTA_SERVER_URL
#define OTA_SERVER_URL "http://your-server.com/firmware"
#endif

#ifndef OTA_VERSION_URL
#define OTA_VERSION_URL "http://your-server.com/version"
#endif

#ifndef OTA_CHECK_INTERVAL
#define OTA_CHECK_INTERVAL 3600  // 1 hour in seconds
#endif

// WiFi Configuration for OTA (if needed)
#ifndef WIFI_SSID
#define WIFI_SSID "YourWiFiSSID"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD "YourWiFiPassword"
#endif

// OTA Features Configuration
#define OTA_MAX_RETRIES 3
#define OTA_TIMEOUT_MS 30000
#define OTA_BUFFER_SIZE 1024

// Debug Options
#define OTA_DEBUG_ENABLED 1

#if OTA_DEBUG_ENABLED
#define OTA_DEBUG(x) Serial.println("[OTA] " + String(x))
#define OTA_DEBUG_F(x, ...) Serial.printf("[OTA] " x "\n", ##__VA_ARGS__)
#else
#define OTA_DEBUG(x)
#define OTA_DEBUG_F(x, ...)
#endif

#endif // OTA_CONFIG_H