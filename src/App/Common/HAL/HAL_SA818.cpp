#include "HAL.h"
#include <Arduino.h>
#include "SA818.h"          // New SA818 driver
#include "App/Common/DataProc/DataProc.h" // Include DataProc

/* Used Pins */
#define SA818_BAUD 9600 // 固定 SA818 波特率為 9600

// Configuration is now in HAL_Def.h to ensure sync with RadioModel
#include "SA818_Channels.h" // SA818 頻道配置

#define SA_RX CONFIG_SA818_RX_PIN
#define SA_TX CONFIG_SA818_TX_PIN

//VBAT Pin 8 , GND Pin 9,10

static SA818 sa818; // Our new SA818 object instance
static HardwareSerial& sa818_serial = Serial2; // Use Serial2 for SA818
static Account* sa818_account = nullptr; // DataProc account for SA818

// PTT (Push To Talk) 變數
static bool ptt_initialized = false;
static bool ptt_state = false;         // 當前 PTT 狀態
static bool prev_ptt_state = false;    // 前一次 PTT 狀態

// SA818 state variables
static int16_t g_sa818_rssi = -999;
static SA818_PowerMode current_power_mode = SA818_LOW_POWER;  // 預設低功率模式
static int current_channel = 1;                               // 預設頻道 1

static uint8_t g_sa818_volume = 5;    // 預設音量
static uint8_t g_sa818_ctcss = 0;     // 預設 CTCSS OFF (0=OFF, 1-38=tone)
static uint8_t g_sa818_squelch = 4;   // 預設靜噪 4

// Helper function to notify DataProc subscribers
static void SA818_Notify()
{
    if (sa818_account)
    {
        SA818_Info_t info;
        HAL::SA818_GetInfo(&info);
        sa818_account->Notify("SA818", &info, sizeof(info));
        // Serial.println("HAL_SA818: Notified DataProc");
    }
}

void HAL::SA818_Init()
{
    Serial.println("SA818_Init: Starting SA818 module with new driver...");

    // Initialize control pins
    pinMode(CONFIG_SA818_PD_PIN, OUTPUT);
    pinMode(CONFIG_SA818_HL_PIN, OUTPUT);

    // Reset the module by toggling the Power Down pin to ensure a clean start
    Serial.println("SA818_Init: Resetting module via PD pin...");
    digitalWrite(CONFIG_SA818_PD_PIN, LOW);  // Power Down (Sleep)
    delay(500);
    digitalWrite(CONFIG_SA818_PD_PIN, HIGH); // Normal Operation
    delay(1500); // Give module time to boot after reset

    digitalWrite(CONFIG_SA818_HL_PIN, LOW);  // Set to low power mode (0.5W)

    // Initialize serial port for SA818
    sa818_serial.begin(SA818_BAUD, SERIAL_8N1, CONFIG_SA818_RX_PIN, CONFIG_SA818_TX_PIN);

    // Initialize the SA818 driver
    bool connected = false;
    for(int i = 0; i < 5; i++) {
        if(sa818.begin(sa818_serial, CONFIG_SA818_PTT_PIN)) {
            connected = true;
            break;
        }
        delay(200);
    }

    if (connected) {
        Serial.println("SA818_Init: Connection successful.");

        // Force reset internal state to defaults to ensure consistency
        current_power_mode = SA818_LOW_POWER;
        current_channel = 1;
        Serial.println("SA818_Init: Resetting state to Low Power, Channel 1");

        // Apply default settings
        // NOTE: The old implementation used CTCSS index. The new one uses frequency.
        // We assume 0 for now. A lookup table will be needed for full functionality.
        float frequency = getSA818Frequency(current_power_mode, current_channel);
        
        // Fix: If we are in VHF mode but got a UHF frequency (likely from default config), override it.
        #if !defined(DRA818_CONFIG_UHF) || (DRA818_CONFIG_UHF == 0)
        if (frequency > 200.0) {
            frequency = 144.8000; // Default to a standard VHF frequency
            Serial.println("SA818_Init: VHF Mode detected with UHF freq. Overriding to 144.8000 MHz.");
        }
        #endif

        if (sa818.setGroup(frequency, frequency, 0, g_sa818_squelch)) {
            Serial.println("SA818_Init: Group configuration successful.");
        } else {
            Serial.println("SA818_Init: Group configuration FAILED! Module may not transmit.");
        }

        if (sa818.setVolume(g_sa818_volume)) {
            Serial.println("SA818_Init: Volume set successful.");
        } else {
            Serial.println("SA818_Init: Volume set FAILED!");
        }

        Serial.printf("SA818_Init: Set to Freq: %.4f MHz, Vol: %d, SQ: %d\n",
                      frequency, g_sa818_volume, g_sa818_squelch);
    } else {
        Serial.println("SA818_Init: *** CONNECTION FAILED! Check wiring and power. ***");
    }

    // Initialize DataProc Account
    sa818_account = new Account("SA818_HAL", DataProc::Center(), 0, nullptr);
    
    Serial.println("SA818_Init: Done...");
}

// This function is called periodically by HAL_Update
void HAL::SA818_Update()
{
    int16_t new_rssi = sa818.scanRSSI();
    // Only notify if RSSI changed significantly or periodically? 
    // For now, let's update global variable. 
    // If we want real-time UI updates, we should notify.
    // To avoid flooding, maybe check if value changed beyond a threshold?
    
    if (abs(new_rssi - g_sa818_rssi) > 2 || new_rssi == -999) {
        g_sa818_rssi = new_rssi;
        SA818_Notify();
    } else {
        g_sa818_rssi = new_rssi;
    }
}

void HAL::SA818_GetInfo(SA818_Info_t* info)
{
    //Serial.println("SA818_GetInfo");
    memset(info, 0, sizeof(SA818_Info_t));

    // This function should be updated to get real values from the `sa818` object.
    // For now, we return a mix of stored values and hardcoded defaults.
    info->channel = current_channel;
    info->freq_rx = SA818_GetCurrentFrequency();
    info->freq_tx = SA818_GetCurrentFrequency();
    info->squelch = g_sa818_squelch;
    info->volume = g_sa818_volume;
    info->ctcss_rx = g_sa818_ctcss;
    info->ctcss_tx = g_sa818_ctcss;
    info->rssi = g_sa818_rssi; // Add the new RSSI value
    // info->BW and info->SQ are from the old struct, can be mapped if needed.
}

// PTT (Push To Talk) 功能實現

void HAL::PTT_Init() {
    Serial.printf("PTT_Init: Configuring PTT Button GPIO %d\n", CONFIG_PTT_PIN);
    
    // 配置 PTT 引腳為輸入模式，啟用內部上拉
    pinMode(CONFIG_PTT_PIN, INPUT_PULLUP);
    
    // 初始化狀態
    ptt_state = false;
    prev_ptt_state = false;
    ptt_initialized = true;
    
    Serial.println("PTT_Init: PTT system initialized");
}

void HAL::PTT_Update() {
    if (!ptt_initialized) return;

    // PTT 按鍵通常是低電平觸發（按下時接地）
    bool current_reading = (digitalRead(CONFIG_PTT_PIN) == LOW);

    if (current_reading != ptt_state) {
        // 狀態改變，執行動作
        ptt_state = current_reading;
        
        if (ptt_state) {
            PTT_SetTransmit(true);
        } else {
            PTT_SetTransmit(false);
        }
    }
}

bool HAL::PTT_IsPressed() {
    return ptt_state;
}

void HAL::PTT_SetTransmit(bool enable) {
    
    if (enable) {
        Serial.printf("PTT: Enabling transmission mode (GPIO %d LOW)\n", CONFIG_SA818_PTT_PIN);
        sa818.transmit(true);
        SA818_Notify(); // Notify state change
    } else {
        Serial.println("PTT: Disabling transmission mode (receive mode)");
        sa818.transmit(false);
        SA818_Notify(); // Notify state change
    }
}

// SA818 控制引腳功能實現

void HAL::SA818_SetPowerDown(bool powerDown) {
    // PD 引腳：LOW = Power Down mode, HIGH = Normal operation
    digitalWrite(CONFIG_SA818_PD_PIN, powerDown ? LOW : HIGH);
    Serial.printf("SA818 PD pin set to %s (Power %s)\n", 
                  powerDown ? "LOW" : "HIGH", 
                  powerDown ? "Down" : "Normal");
    
    if (powerDown) {
        delay(100);  // 給 SA818 時間進入省電模式
    } else {
        delay(1000); // 從省電模式恢復需要更長時間
    }
}

void HAL::SA818_SetHighLowPower(bool highPower) {
    // H/L 引腳：LOW = Low Power (0.5W), HIGH = High Power (1W)
    digitalWrite(CONFIG_SA818_HL_PIN, highPower ? HIGH : LOW);
    Serial.printf("SA818 H/L pin set to %s (%s Power - %s)\n", 
                  highPower ? "HIGH" : "LOW", 
                  highPower ? "High" : "Low",
                  highPower ? "1W" : "0.5W");
}

bool HAL::SA818_GetPowerDown() {
    return digitalRead(CONFIG_SA818_PD_PIN) == LOW;
}

bool HAL::SA818_GetHighLowPower() {
    return digitalRead(CONFIG_SA818_HL_PIN) == HIGH;
}

// SA818 頻道管理功能

bool HAL::SA818_UpdateGroup() {
    
    float frequency = getSA818Frequency(current_power_mode, current_channel);
    if (frequency == 0.0) {
        Serial.printf("SA818_UpdateGroup: Failed to get frequency\n");
        return false;
    }
    
    // group(bandwidth, freq_tx, freq_rx, ctcss_tx, squelch, ctcss_rx)
    Serial.printf("SA818_UpdateGroup: freq=%.4f, CTCSS=%d, SQ=%d\n", 
                  frequency, g_sa818_ctcss, g_sa818_squelch);
    
    bool result = sa818.setGroup(frequency, frequency, 0, g_sa818_squelch);
    
    if (result) {
        Serial.println("SA818_UpdateGroup: Success");
        SA818_Notify();
        return true;
    } else {
        Serial.println("SA818_UpdateGroup: Failed to set group parameters!");
        return false;
    }
}

bool HAL::SA818_SetChannel(int channel, SA818_PowerMode powerMode) {
    
    if (channel < SA818_MIN_CHANNEL || channel > SA818_MAX_CHANNEL) {
        Serial.printf("SA818_SetChannel: Invalid channel %d (valid range: %d-%d)\n", 
                      channel, SA818_MIN_CHANNEL, SA818_MAX_CHANNEL);
        return false;
    }
    
    float frequency = getSA818Frequency(powerMode, channel);
    if (frequency == 0.0) {
        Serial.printf("SA818_SetChannel: Failed to get frequency for channel %d\n", channel);
        return false;
    }
    
    // 設定硬體功率模式引腳
    SA818_SetHighLowPower(powerMode == SA818_HIGH_POWER);
    
    // 配置 SA818 頻率
    Serial.printf("SA818_SetChannel: Setting channel %d (%s) to %.4f MHz, CTCSS=%d, SQ=%d\n", 
                  channel, getPowerModeName(powerMode), frequency, g_sa818_ctcss, g_sa818_squelch);
    
    // 使用 DRA818 庫設定頻率和 CTCSS
    bool result = sa818.setGroup(frequency, frequency, 0, g_sa818_squelch);
    
    if (result) {
        current_channel = channel;
        current_power_mode = powerMode;
        Serial.printf("SA818_SetChannel: Successfully set to channel %d (%s, %.4f MHz)\n", 
                      channel, getPowerModeName(powerMode), frequency);
        SA818_Notify();
        return true;
    } else {
        Serial.printf("SA818_SetChannel: Failed to set channel %d (Command rejected)\n", channel);
        return false;
    }
}

int HAL::SA818_GetChannel() {
    return current_channel;
}

SA818_PowerMode HAL::SA818_GetPowerMode() {
    return current_power_mode;
}

float HAL::SA818_GetCurrentFrequency() {
    return getSA818Frequency(current_power_mode, current_channel);
}

bool HAL::SA818_NextChannel() {
    int next_channel = current_channel + 1;
    if (next_channel > SA818_MAX_CHANNEL) {
        next_channel = SA818_MIN_CHANNEL;
    }
    return SA818_SetChannel(next_channel, current_power_mode);
}

bool HAL::SA818_PreviousChannel() {
    int prev_channel = current_channel - 1;
    if (prev_channel < SA818_MIN_CHANNEL) {
        prev_channel = SA818_MAX_CHANNEL;
    }
    return SA818_SetChannel(prev_channel, current_power_mode);
}

bool HAL::SA818_TogglePowerMode() {
    SA818_PowerMode new_mode = (current_power_mode == SA818_LOW_POWER) ? 
                               SA818_HIGH_POWER : SA818_LOW_POWER;
    return SA818_SetChannel(current_channel, new_mode);
}

void HAL::SA818_GetChannelInfo(SA818_ChannelInfo_t* info) {
    if (!info) return;
    
    info->channel = current_channel;
    info->powerMode = current_power_mode;
    info->frequency = getSA818Frequency(current_power_mode, current_channel);
    
    // 複製模式名稱和頻率範圍描述
    strncpy(info->powerModeName, getPowerModeName(current_power_mode), sizeof(info->powerModeName) - 1);
    info->powerModeName[sizeof(info->powerModeName) - 1] = '\0';
    
    strncpy(info->frequencyRange, getFrequencyRange(current_power_mode), sizeof(info->frequencyRange) - 1);
    info->frequencyRange[sizeof(info->frequencyRange) - 1] = '\0';
}

void HAL::SA818_SetVolume(uint8_t vol)
{
    if (vol < 1) vol = 1;
    if (vol > 8) vol = 8;
    g_sa818_volume = vol;
    sa818.setVolume(vol); // 實際設定到 SA818
    Serial.printf("HAL_SA818: Set volume to %d\n", vol);
    SA818_Notify();
}

uint8_t HAL::SA818_GetVolume()
{
    return g_sa818_volume;
}

void HAL::SA818_SetCTCSS(uint8_t ctcss)
{
    // CTCSS: 0=OFF, 1-38=tone index
    if (ctcss > 38) ctcss = 38;
    g_sa818_ctcss = ctcss;
    Serial.printf("HAL_SA818: Set CTCSS to %d\n", ctcss);
    
    // 更新 SA818 設定
    SA818_UpdateGroup();
}

uint8_t HAL::SA818_GetCTCSS()
{
    return g_sa818_ctcss;
}

void HAL::SA818_SetSquelch(uint8_t squelch)
{
    // Squelch: 0-8
    if (squelch > 8) squelch = 8;
    g_sa818_squelch = squelch;
    Serial.printf("HAL_SA818: Set Squelch to %d\n", squelch);
    
    // 更新 SA818 設定
    SA818_UpdateGroup();
}

uint8_t HAL::SA818_GetSquelch()
{
    return g_sa818_squelch;
}