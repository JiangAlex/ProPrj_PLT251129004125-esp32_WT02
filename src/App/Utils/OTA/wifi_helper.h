#ifndef WIFI_HELPER_H
#define WIFI_HELPER_H

#include <WiFi.h>
#include "ota_config.h"

class WiFiHelper {
private:
    static bool isConnected;
    static unsigned long lastConnectionAttempt;
    static const unsigned long CONNECTION_TIMEOUT = 10000; // 10 seconds
    
public:
    static bool init();
    static bool connect();
    static bool isWiFiConnected();
    static void disconnect();
    static void printStatus();
    static String getIPAddress();
};

#endif // WIFI_HELPER_H