#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include <ArduinoJson.h>

typedef void (*OTAProgressCallback)(int percent);

class OTAUpdater {
private:
    String currentVersion;
    String serverURL;
    String versionURL;
    unsigned long lastCheckTime;
    unsigned long checkInterval;
    bool autoCheckEnabled;
    OTAProgressCallback progressCb;
    
    bool checkWiFiConnection();
    bool downloadAndInstallFirmware(const String& firmwareURL);
    void rebootDevice();
    int compareVersions(const String& version1, const String& version2);

public:
    OTAUpdater();
    void begin(const String& version, const String& serverUrl, const String& versionUrl, unsigned long interval = 3600000);
    void enableAutoCheck(bool enable);
    bool checkForUpdates();
    String getRemoteVersion();
    void handleAutoCheck();
    bool performUpdate();
    void setProgressCallback(OTAProgressCallback cb);
    void setCurrentVersion(const String& version);
    String getLastCheckTime();
};

extern OTAUpdater otaUpdater;

#endif