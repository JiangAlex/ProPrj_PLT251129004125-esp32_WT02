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

    // GPX file selection
    int GetGPXFileCount();
    int GetGPXSelected();
    void SetGPXSelected(int idx);
    String GetGPXPath();

    // Static access for Profile page
    static String activeGPXPath;

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
    
    // 環境數據
    float currentTemp;
    float currentPress;
    
    // GPS tracking
    double prevLat;
    double prevLon;
    float prevAlt;
    bool lastValid;

    // GPX file selection
    int gpxFileCount;
    int gpxSelected;

    void UpdateSensors();
};

}

#endif