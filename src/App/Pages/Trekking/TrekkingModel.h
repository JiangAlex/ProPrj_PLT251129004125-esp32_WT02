#ifndef __TREKKING_MODEL_H
#define __TREKKING_MODEL_H

#include "App/Common/DataProc/DataProc.h"
#include "App/Common/HAL/HAL.h"

namespace Page {

class TrekkingModel {
public:
    TrekkingModel();
    void Init();
    void Deinit();
    void Update(); // 定期更新數據

    // 狀態控制
    void StartRecord();
    void PauseRecord();
    void StopRecord();
    bool IsRecording();

    // 數據獲取
    uint32_t GetTimeMs();
    float GetDistanceKm();
    float GetAltitude();
    float GetTemperature();
    float GetPressure();
    float GetAscent();
    uint32_t GetSteps();

private:
    Account* account;
    
    // 狀態變數
    bool isRecording;
    uint32_t startTime;
    uint32_t pauseTime;
    uint32_t totalPauseTime;
    
    // 運動數據
    float currentAlt;
    float startAlt;
    float totalAscent;
    float currentDist;
    uint32_t startSteps;
    uint32_t currentSteps;
    
    // 環境數據
    float currentTemp;
    float currentPress;

    void UpdateSensors();
};

}

#endif