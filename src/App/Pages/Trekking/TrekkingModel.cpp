#include "TrekkingModel.h"
#include <Arduino.h>
#include <SPIFFS.h>

using namespace Page;

String TrekkingModel::activeGPXPath = "";
TrekkingModel::LivePoint TrekkingModel::livePts[TrekkingModel::LIVE_MAX_PTS];
int TrekkingModel::livePtCount = 0;
float TrekkingModel::liveMaxTime = 0;

TrekkingModel::TrekkingModel()
{
    account = nullptr;
    isRecording = false;
    startTime = 0;
    pauseTime = 0;
    totalPauseTime = 0;
    
    currentAlt = 0.0f;
    startAlt = 0.0f;
    totalAscent = 0.0f;
    currentDist = 0.0f;
    currentTemp = 0.0f;
    currentPress = 0.0f;
    prevLat = 0.0;
    prevLon = 0.0;
    prevAlt = 0.0f;
    lastValid = false;
    gpxFileCount = 0;
    gpxSelected = 0;
}

void TrekkingModel::Init()
{
    account = new Account("TrekkingModel", DataProc::Center(), 0, this);
    // 訂閱感測器數據 (待 DataProc 完善後啟用)
    // account->Subscribe("GPS");
    // account->Subscribe("ENV");
    
    // 初始化讀取一次感測器
    UpdateSensors();
    startAlt = currentAlt;
}

void TrekkingModel::Deinit()
{
    if (account) {
        delete account;
        account = nullptr;
    }
}

void TrekkingModel::Update()
{
    UpdateSensors();
    
    if (isRecording && lastValid) {
        // Record live point every 10 seconds
        float elapsed = GetTimeMs() / 1000.0f;
        if (livePtCount == 0 || (elapsed - liveMaxTime >= 10.0f)) {
            if (livePtCount < LIVE_MAX_PTS) {
                livePts[livePtCount].time_sec = elapsed;
                livePts[livePtCount].alt = currentAlt;
                livePtCount++;
                liveMaxTime = elapsed;
            }
        }

        // Accumulate distance from GPS
        GPS_Info_t gpsInfo;
        if (HAL::GPS_GetInfo(&gpsInfo) && gpsInfo.isValid) {
            if (prevLat != 0.0 || prevLon != 0.0) {
                double d = HAL::GPS_GetDistanceOffset(&gpsInfo, prevLon, prevLat);
                if (d > 2.0) { // Filter GPS noise (>2m)
                    currentDist += d / 1000.0; // to km
                    // Ascent tracking
                    if (gpsInfo.altitude > prevAlt + 1.0) {
                        totalAscent += (gpsInfo.altitude - prevAlt);
                    }
                    prevLat = gpsInfo.latitude;
                    prevLon = gpsInfo.longitude;
                    prevAlt = gpsInfo.altitude;
                }
            } else {
                prevLat = gpsInfo.latitude;
                prevLon = gpsInfo.longitude;
                prevAlt = gpsInfo.altitude;
            }
        }
    }
}

void TrekkingModel::StartRecord()
{
    if (!isRecording) {
        if (startTime == 0) {
            startTime = millis();
            startAlt = currentAlt;
            totalAscent = 0;
            currentDist = 0;
            prevLat = 0.0;
            prevLon = 0.0;
            prevAlt = currentAlt;
        } else {
            if (pauseTime > 0) {
                totalPauseTime += (millis() - pauseTime);
                pauseTime = 0;
            }
        }
        isRecording = true;
    }
}

void TrekkingModel::PauseRecord()
{
    if (isRecording) {
        isRecording = false;
        pauseTime = millis();
    }
}

void TrekkingModel::StopRecord()
{
    isRecording = false;
    startTime = 0;
    pauseTime = 0;
    totalPauseTime = 0;
    livePtCount = 0;
    liveMaxTime = 0;
}

bool TrekkingModel::IsRecording()
{
    return isRecording;
}

uint32_t TrekkingModel::GetTimeMs()
{
    if (startTime == 0) return 0;
    
    uint32_t now = millis();
    if (isRecording) {
        return now - startTime - totalPauseTime;
    } else {
        // 暫停狀態下，時間停留在暫停那一刻
        return pauseTime - startTime - totalPauseTime;
    }
}

float TrekkingModel::GetDistanceKm() { return currentDist; }
float TrekkingModel::GetAltitude() { return currentAlt; }
float TrekkingModel::GetTemperature() { return currentTemp; }
float TrekkingModel::GetPressure() { return currentPress; }
float TrekkingModel::GetAscent() { return totalAscent; }

float TrekkingModel::GetSpeed() {
    GPS_Info_t info;
    if (HAL::GPS_GetInfo(&info)) return info.speed;
    return 0.0f;
}

int TrekkingModel::GetSatellites() {
    GPS_Info_t info;
    HAL::GPS_GetInfo(&info);
    return info.satellites;
}

int TrekkingModel::GetGPXFileCount() {
    gpxFileCount = 0;
    File idx = SPIFFS.open("/gpx/index.txt", "r");
    if (idx) {
        while (idx.available()) {
            idx.readStringUntil('\n');
            gpxFileCount++;
        }
        idx.close();
    }
    return gpxFileCount;
}

int TrekkingModel::GetGPXSelected() { return gpxSelected; }

void TrekkingModel::SetGPXSelected(int idx) {
    int count = GetGPXFileCount();
    if (count == 0) return;
    if (idx < 0) idx = count - 1;
    if (idx >= count) idx = 0;
    gpxSelected = idx;
    activeGPXPath = GetGPXPath();
}

String TrekkingModel::GetGPXPath() {
    File idx = SPIFFS.open("/gpx/index.txt", "r");
    if (!idx) return "";
    int lineNum = 0;
    while (idx.available()) {
        String line = idx.readStringUntil('\n');
        line.trim();
        if (lineNum == gpxSelected) {
            idx.close();
            int num = line.toInt();
            char path[32];
            snprintf(path, sizeof(path), "/gpx/%03d.bin", num);
            return String(path);
        }
        lineNum++;
    }
    idx.close();
    return "";
}

void TrekkingModel::UpdateSensors()
{
    GPS_Info_t gpsInfo;
    if (HAL::GPS_GetInfo(&gpsInfo) && gpsInfo.isValid) {
        currentAlt = gpsInfo.altitude;
        lastValid = true;
    }
    // No temp/pressure sensor yet
    currentTemp = 0.0f;
    currentPress = 0.0f;
}