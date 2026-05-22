#include "Radio.h"

using namespace Page;

void Radio::StopScanning() {
    if (scanTimer) {
        lv_timer_del(scanTimer);
        scanTimer = nullptr;
    }
    View.SetScanning(false);
    Serial.println("[Radio] Scanning stopped");
}

void Radio::onScanTimer(lv_timer_t* timer) {
    Radio* instance = (Radio*)timer->user_data;
    
    // 1. 強制更新 RSSI (從硬體讀取)
    HAL::SA818_Update();
    
    // 2. 檢查當前頻道的 RSSI
    int rssi = instance->Model.GetRSSI();
    // Serial.printf("[Radio] Scan: CH %d, RSSI %d\n", instance->Model.GetChannel(), rssi);

    if (rssi > -90) {
        // 訊號夠強，停止掃描
        instance->StopScanning();
        return;
    }
    
    // 3. 切換到下一個頻道
    int ch = instance->Model.GetChannel();
    ch++;
    if (ch > 20) ch = 1; // 假設最大 20 個頻道
    
    instance->Model.SetChannel(ch);
    
    // 即時更新 UI 顯示
    instance->View.UpdateValue(RadioView::RADIO_ITEM_CH, ch);
    instance->View.UpdateFrequency(instance->Model.GetFrequencyFor(ch, instance->Model.IsHighPower()));
    // 注意：切換頻道後需要一點時間讓 PLL 鎖定和 RSSI 穩定，下一次 timer tick (200ms後) 再檢查 RSSI
}

Radio::Radio()
{
    lastBtnTime = 0;
    lastBtnUp = true;
    lastBtnDown = true;
    lastBtnOK = true;
    scanTimer = nullptr;
}

Radio::~Radio()
{
}

void Radio::onCustomAttrConfig()
{
    SetCustomCacheEnable(false);
    SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);
}

void Radio::onViewLoad()
{
    Model.Init();
    Serial.println("[Radio] onViewLoad");
    View.Create(root);
    
    // 從 Model 初始化 View 的數值
    View.UpdateValue(RadioView::RADIO_ITEM_CH, Model.GetChannel());
    View.UpdateFrequency(Model.GetFrequency());
    View.UpdateValue(RadioView::RADIO_ITEM_CTCSS, Model.GetCTCSSIndex());
    View.UpdateValue(RadioView::RADIO_ITEM_POWER, Model.IsHighPower() ? 1 : 0);
    View.UpdateValue(RadioView::RADIO_ITEM_RSSI, Model.GetRSSI());
    View.UpdateValue(RadioView::RADIO_ITEM_VOL, Model.GetVolume());
    View.UpdateValue(RadioView::RADIO_ITEM_CQL, Model.GetSquelch()); // CQL is SQ
    
    Serial.printf("[Radio] Loaded SA818: CH=%d, Freq=%.4f MHz, Power=%s, Vol=%d\n",
                  Model.GetChannel(), Model.GetFrequency(),
                  Model.IsHighPower() ? "HIGH" : "LOW", Model.GetVolume());
    
    // 設置 GPIO 為輸入
    pinMode(RADIO_BTN_UP, INPUT_PULLUP);
    pinMode(RADIO_BTN_DOWN, INPUT_PULLUP);
    pinMode(RADIO_BTN_OK, INPUT_PULLUP);
    
    // 創建定時器檢查按鍵
    btnTimer = lv_timer_create(onTimer, 50, this);  // Check every 50ms for responsiveness
}

void Radio::onViewDidLoad()
{
    Serial.println("[Radio] onViewDidLoad");
}

void Radio::onViewWillAppear()
{
    Model.PlayMusic("Radio");
    if (View.ui.anim_timeline != nullptr) {
        lv_anim_timeline_start(View.ui.anim_timeline);
    }
    Serial.println("[Radio] onViewWillAppear");
}

void Radio::onViewDidAppear()
{
    Serial.println("[Radio] onViewDidAppear");
}

void Radio::onViewWillDisappear()
{
    Serial.println("[Radio] onViewWillDisappear");
    
    // 刪除定時器（必須在頁面消失前刪除）
    if (btnTimer) {
        lv_timer_del(btnTimer);
        btnTimer = nullptr;
        Serial.println("[Radio] Timer deleted");
    }
    
    // 離開頁面時停止掃描
    if (scanTimer) {
        StopScanning();
    }
}

void Radio::onViewDidDisappear()
{
    Serial.println("[Radio] onViewDidDisappear");
}

void Radio::onViewUnload()
{
    // 注意: 此函數不會被 PageManager 調用
    // Timer 清理已移到 onViewWillDisappear
    View.Delete();
    Serial.println("[Radio] onViewUnload");
    Model.Deinit();
}

void Radio::onViewDidUnload()
{
    Serial.println("[Radio] onViewDidUnload");
}

void Radio::onTimer(lv_timer_t *timer)
{
    Radio *instance = (Radio *)timer->user_data;
    if (!instance) return;
    
    uint32_t now = millis();

    // Update view if model data has changed (e.g. from DataProc)
    if (instance->Model.IsDirty()) {
        SA818_Info_t info;
        instance->Model.GetSA818Info(&info);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_CH, info.channel);
        instance->View.UpdateFrequency(info.freq_rx);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_CTCSS, info.ctcss_rx);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_POWER, HAL::SA818_GetPowerMode() == SA818_HIGH_POWER ? 1 : 0);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_RSSI, info.rssi);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_VOL, info.volume);
        instance->View.UpdateValue(RadioView::RADIO_ITEM_CQL, info.squelch);
        instance->Model.ClearDirty();
    }
    
    // 防抖檢查
    if (now - instance->lastBtnTime < instance->DEBOUNCE_MS) {
        return;
    }
    
    // 讀取按鍵狀態（低電平有效）
    bool btnUp = digitalRead(RADIO_BTN_UP);
    bool btnDown = digitalRead(RADIO_BTN_DOWN);
    bool btnOK = digitalRead(RADIO_BTN_OK);
    
    // 如果正在掃描，按下任意鍵停止掃描
    if (instance->scanTimer != nullptr) {
        if ((instance->lastBtnUp && !btnUp) || 
            (instance->lastBtnDown && !btnDown) || 
            (instance->lastBtnOK && !btnOK)) {
            
            instance->StopScanning();
            instance->lastBtnTime = now;
            
            // 更新按鍵狀態，消耗掉這次按鍵事件
            instance->lastBtnUp = btnUp;
            instance->lastBtnDown = btnDown;
            instance->lastBtnOK = btnOK;
            return;
        }
    }
    
    // 檢測按鍵下降沿（從高到低）
    if (instance->lastBtnUp && !btnUp) {
        instance->handleButtonUp();
        instance->lastBtnTime = now;
    }
    if (instance->lastBtnDown && !btnDown) {
        instance->handleButtonDown();
        instance->lastBtnTime = now;
        Serial.println("[Radio] After handleButtonDown");
    }
    if (instance->lastBtnOK && !btnOK) {
        instance->okPressStart = now;
        instance->okLongHandled = false;
    }
    if (!btnOK && instance->okPressStart > 0 && !instance->okLongHandled && (now - instance->okPressStart > 3000)) {
        instance->Manager->Pop();
        instance->okLongHandled = true;
    }
    if (btnOK && instance->okPressStart > 0) {
        if (!instance->okLongHandled) {
            instance->handleButtonOK();
        }
        instance->okPressStart = 0;
        instance->lastBtnTime = now;
    }
    
    // 保存按鍵狀態
    //Serial.println("[Radio] Saving button states");
    instance->lastBtnUp = btnUp;
    instance->lastBtnDown = btnDown;
    instance->lastBtnOK = btnOK;
    //Serial.println("[Radio] onTimer done");
}

void Radio::handleButtonUp()
{
    Serial.println("[Radio] BTN_UP pressed");
    
    if (View.IsInFuncArea()) {
        // 在功能區，UP 返回資訊區最後一項（CQL）
        View.ExitFuncArea();
        View.SetSelected(RadioView::RADIO_ITEM_CQL);
    }
    else if (View.IsEditMode()) {
        // 編輯模式：增加數值並同步到 Model
        int idx = View.GetSelected();
        int val = View.GetValue(idx);
        int newVal = val;
        
        switch (idx) {
            case RadioView::RADIO_ITEM_CH:
                newVal = val + 1;
                if (newVal > 20) newVal = 20;
                View.UpdateValue(idx, newVal);
                Model.SetChannel(newVal);
                View.UpdateFrequency(Model.GetFrequencyFor(newVal, View.GetValue(RadioView::RADIO_ITEM_POWER) == 1));
                break;
            case RadioView::RADIO_ITEM_CTCSS:
                newVal = val + 1;
                if (newVal > 38) newVal = 0;  // Wrap around, 0 is OFF
                View.UpdateValue(idx, newVal);
                Model.SetCTCSSIndex(newVal);
                break;
            case RadioView::RADIO_ITEM_POWER:
                newVal = 1;  // HIGH
                View.UpdateValue(idx, newVal);
                Model.SetHighPower(true);
                View.UpdateFrequency(Model.GetFrequencyFor(View.GetValue(RadioView::RADIO_ITEM_CH), true));
                break;
            case RadioView::RADIO_ITEM_VOL:
                newVal = val + 1;
                if (newVal > 8) newVal = 8;
                View.UpdateValue(idx, newVal);
                Model.SetVolume(newVal);
                break;
            case RadioView::RADIO_ITEM_CQL:
                newVal = val + 1;
                if (newVal > 8) newVal = 1; // Wrap around
                View.UpdateValue(idx, newVal);
                Model.SetSquelch(newVal);
                break;
            default:
                break;
        }
    }
    else {
        // 導航模式：向上移動
        int idx = View.GetSelected();
        if (idx > 0) {
            View.SetSelected(idx - 1);
        }
    }
}

void Radio::handleButtonDown()
{
    Serial.println("[Radio] BTN_DOWN pressed");
    
    if (View.IsInFuncArea()) {
        // 在功能區切換 SCAN / BACK
        int current = View.GetFuncSelected();
        int next = (current == RadioView::RADIO_FUNC_SCAN) ? RadioView::RADIO_FUNC_BACK : RadioView::RADIO_FUNC_SCAN;
        View.SetFuncSelected(next);
    }
    else if (View.IsEditMode()) {
        // 編輯模式：減少數值並同步到 Model
        int idx = View.GetSelected();
        int val = View.GetValue(idx);
        int newVal = val;
        
        switch (idx) {
            case RadioView::RADIO_ITEM_CH:
                newVal = val - 1;
                if (newVal < 1) newVal = 1;
                View.UpdateValue(idx, newVal);
                Model.SetChannel(newVal);
                View.UpdateFrequency(Model.GetFrequencyFor(newVal, View.GetValue(RadioView::RADIO_ITEM_POWER) == 1));
                break;
            case RadioView::RADIO_ITEM_CTCSS:
                newVal = val - 1;
                if (newVal < 0) newVal = 38;  // Wrap around
                View.UpdateValue(idx, newVal);
                Model.SetCTCSSIndex(newVal);
                break;
            case RadioView::RADIO_ITEM_POWER:
                newVal = 0;  // LOW
                View.UpdateValue(idx, newVal);
                Model.SetHighPower(false);
                View.UpdateFrequency(Model.GetFrequencyFor(View.GetValue(RadioView::RADIO_ITEM_CH), false));
                break;
            case RadioView::RADIO_ITEM_VOL:
                newVal = val - 1;
                if (newVal < 1) newVal = 1;
                View.UpdateValue(idx, newVal);
                Model.SetVolume(newVal);
                break;
            case RadioView::RADIO_ITEM_CQL:
                newVal = val - 1;
                if (newVal < 1) newVal = 8; // Wrap around
                View.UpdateValue(idx, newVal);
                Model.SetSquelch(newVal);
                break;
            default:
                break;
        }
    }
    else {
        // 導航模式：向下移動
        int idx = View.GetSelected();
        if (idx < View.GetItemCount() - 1) {
            View.SetSelected(idx + 1);
        }
        else if (idx == RadioView::RADIO_ITEM_CQL) {
            // 在 CQL（最後一項），按 DOWN 進入功能區
            View.EnterFuncArea();
        }
    }
}

void Radio::handleButtonOK()
{
    Serial.println("[Radio] BTN_OK pressed");
    
    if (View.IsInFuncArea()) {
        int func = View.GetFuncSelected();
        if (func == RadioView::RADIO_FUNC_BACK) {
            Serial.println("[Radio] BACK selected, returning to StartUp");
            Manager->Pop();
        } else if (func == RadioView::RADIO_FUNC_SCAN) {
            if (scanTimer == nullptr) {
                Serial.println("[Radio] Starting scan...");
                View.SetScanning(true);
                // 創建 200ms 的掃描定時器
                scanTimer = lv_timer_create(onScanTimer, 200, this);
            } else {
                // 如果已經在掃描，再次按下則停止
                StopScanning();
            }
        }
    }
    else if (View.IsEditMode()) {
        // 退出編輯模式，進入導航模式
        View.SetEditMode(false);
        Serial.println("[Radio] Exited edit mode");
    }
    else {
        // 進入編輯模式
        int idx = View.GetSelected();
        
        // RSSI 是只讀的，不能編輯
        if (idx == RadioView::RADIO_ITEM_RSSI) {
            Serial.println("[Radio] RSSI is read-only");
            return;
        }
        
        View.SetEditMode(true);
        Serial.printf("[Radio] Entered edit mode for item %d\n", idx);
    }
}

void Radio::onEvent(lv_event_t *event)
{
    Radio *instance = (Radio *)lv_event_get_user_data(event);
    LV_ASSERT_NULL(instance);

    lv_obj_t *obj = lv_event_get_current_target(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (obj == instance->root)
    {
        if (code == LV_EVENT_LEAVE)
        {
            // instance->Manager->Pop();
        }
    }
}
