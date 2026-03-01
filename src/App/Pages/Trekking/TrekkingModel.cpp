#include "TrekkingModel.h"
#include <Arduino.h>

using namespace Page;

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
    startSteps = 0;
    currentSteps = 0;
    currentTemp = 0.0f;
    currentPress = 0.0f;
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
    startSteps = currentSteps;
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
    
    if (isRecording) {
        // 這裡實作距離和爬升的累計邏輯
        // 範例：簡單模擬數據變化
        // currentDist += 0.001f; 
        
        if (currentAlt > startAlt) {
            // 簡單計算爬升 (實際應考慮雜訊過濾)
            // totalAscent += (currentAlt - lastAlt);
        }
    }
}

void TrekkingModel::StartRecord()
{
    if (!isRecording) {
        if (startTime == 0) {
            // 第一次開始
            startTime = millis();
            startAlt = currentAlt;
            startSteps = currentSteps;
            totalAscent = 0;
            currentDist = 0;
        } else {
            // 從暫停恢復
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
    // TODO: 可以在這裡儲存活動紀錄
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

uint32_t TrekkingModel::GetSteps() {
    if (currentSteps >= startSteps)
        return currentSteps - startSteps;
    return 0;
}

void TrekkingModel::UpdateSensors()
{
    // 獲取 GPS 數據
    // GPS_Info_t gpsInfo;
    // if (HAL::GPS_GetInfo(&gpsInfo)) {
    //     currentAlt = gpsInfo.altitude;
    // } else {
        // 模擬數據
        currentAlt = 100.0f; 
    // }

    // 獲取其他感測器數據 (待 HAL 完善)
    currentTemp = 25.5f;   // Placeholder
    currentPress = 1013.0f; // Placeholder
    
    // 模擬步數增加
    // currentSteps++;
}