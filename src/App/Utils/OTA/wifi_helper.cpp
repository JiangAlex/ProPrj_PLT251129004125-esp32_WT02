#include "wifi_helper.h"

bool WiFiHelper::isConnected = false;
unsigned long WiFiHelper::lastConnectionAttempt = 0;

bool WiFiHelper::init() {
    WiFi.mode(WIFI_STA);
    OTA_DEBUG("WiFi initialized in STA mode");
    return true;
}

bool WiFiHelper::connect() {
    if (isWiFiConnected()) {
        return true;
    }
    
    unsigned long currentTime = millis();
    if (currentTime - lastConnectionAttempt < 30000) { // Don't try too often
        return false;
    }
    
    lastConnectionAttempt = currentTime;
    
    OTA_DEBUG_F("Connecting to WiFi: %s", WIFI_SSID);
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < CONNECTION_TIMEOUT) {
        delay(500);
        Serial.print(".");
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        isConnected = true;
        OTA_DEBUG_F("WiFi connected! IP: %s", WiFi.localIP().toString().c_str());
        return true;
    } else {
        isConnected = false;
        OTA_DEBUG("WiFi connection failed");
        return false;
    }
}

bool WiFiHelper::isWiFiConnected() {
    isConnected = (WiFi.status() == WL_CONNECTED);
    return isConnected;
}

void WiFiHelper::disconnect() {
    WiFi.disconnect();
    isConnected = false;
    OTA_DEBUG("WiFi disconnected");
}

void WiFiHelper::printStatus() {
    if (isWiFiConnected()) {
        OTA_DEBUG_F("WiFi Status: Connected to %s", WiFi.SSID().c_str());
        OTA_DEBUG_F("IP Address: %s", WiFi.localIP().toString().c_str());
        OTA_DEBUG_F("Signal Strength: %d dBm", WiFi.RSSI());
    } else {
        OTA_DEBUG("WiFi Status: Disconnected");
    }
}

String WiFiHelper::getIPAddress() {
    if (isWiFiConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}