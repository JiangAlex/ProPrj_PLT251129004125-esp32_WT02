#include "HAL.h"
//#include <HardwareSerial.h>
#include <Arduino.h>
#include "DRA818.h" // uncomment the following line in DRA818.h (#define DRA818_DEBUG)
#include "SA818_Channels.h" // SA818 頻道配置

/* Used Pins */
#define SA818_BAUD          9600    // 固定 SA818 波特率為 9600
const int SERIAL_SPEED = 9600;     // 固定波特率，不可動態修改
#define DRA818_CONFIG_UHF 1        //0: VHF , 1: UHF

#define SA_RX CONFIG_SA818_RX_PIN
#define SA_TX CONFIG_SA818_TX_PIN

//VBAT Pin 8 , GND Pin 9,10 

//HardwareSerial dra_serial(1);
HardwareSerial dra_serial(2);

DRA818 *dra;                // the DRA object once instanciated
float freq;                 // the next frequency to scan

// PTT (Push To Talk) 變數
static bool ptt_initialized = false;
static bool ptt_state = false;         // 當前 PTT 狀態
static bool prev_ptt_state = false;    // 前一次 PTT 狀態

// SA818 頻道管理變數
static SA818_PowerMode current_power_mode = SA818_LOW_POWER;  // 預設低功率模式
static int current_channel = 1;                               // 預設頻道 1

static uint8_t g_sa818_volume = 5;    // 預設音量
static uint8_t g_sa818_ctcss = 0;     // 預設 CTCSS OFF (0=OFF, 1-38=tone)
static uint8_t g_sa818_squelch = 4;   // 預設靜噪 4

void HAL::SA818_Init()
{
    Serial.println("SA818_Init: Starting SA818 module initialization...");
    Serial.print("initializing I/O ... \r\n");
    
    // 初始化 SA818 控制引腳
    pinMode(CONFIG_SA818_PD_PIN, OUTPUT);
    pinMode(CONFIG_SA818_HL_PIN, OUTPUT);
    // CONFIG_SA818_SQ_PIN , CONFIG_SA818_PTT_PIN
    
    // 設定初始狀態
    digitalWrite(CONFIG_SA818_PD_PIN, HIGH);  // PD=HIGH: 正常工作模式 (不是 Power Down)
    digitalWrite(CONFIG_SA818_HL_PIN, LOW);   // H/L=LOW: 低功率模式 (0.5W)
    
    // 檢查引腳配置
    Serial.printf("SA818 UART: ESP32 RX2(GPIO%d) -> SA818 pin17(TX), ESP32 TX2(GPIO%d) -> SA818 pin16(RX)\n", CONFIG_SA818_RX_PIN, CONFIG_SA818_TX_PIN);
    Serial.printf("SA818 Control: PD(GPIO%d)=HIGH, H/L(GPIO%d)=LOW\n", CONFIG_SA818_PD_PIN, CONFIG_SA818_HL_PIN);
    Serial.printf("PTT Pin: GPIO%d\n", CONFIG_SA818_PTT_PIN);
    
    // 清空串列緩衝區
    dra_serial.begin(SERIAL_SPEED,SERIAL_8N1,SA_RX,SA_TX);
    delay(100);
    while(dra_serial.available()) {
        dra_serial.read();
    }
    
    // Add power-on delay to allow SA818 module to initialize
    //Serial.println("Waiting for SA818 module to initialize...");
    //delay(3000);  // 增加到 3 秒
    
    Serial.println("Creating DRA818 object...");
    #if DRA818_CONFIG_UHF
        dra = new DRA818((HardwareSerial*) &dra_serial, SA818_UHF);  // 使用 SA818_UHF 而不是 DRA818_UHF
    #else
        dra = new DRA818((HardwareSerial*) &dra_serial, SA818_VHF);  // 使用 SA818_VHF 而不是 DRA818_VHF
    #endif
    
    #ifdef DRA818_DEBUG
    dra->set_log(&Serial);
    Serial.println("Debug logging enabled");
    #endif
    
    // 固定使用 9600 baud rate 進行串列通信
    Serial.printf("SA818: Using fixed baud rate %d bps\n", SERIAL_SPEED);
    
    // 測試基本連接
    Serial.println("Testing SA818 connection with AT+DMOCONNECT...");
    
    // 清空緩衝區
    while(dra_serial.available()) {
        dra_serial.read();
    }
    
    // 測試 AT+DMOCONNECT 命令
    Serial.print("Command: AT+DMOCONNECT -> ");
    dra_serial.print("AT+DMOCONNECT\r\n");
    dra_serial.flush();
    delay(2000);
    
    // 檢查回應
    char response_buffer[100];
    int buffer_index = 0;
    bool found_dmoconnect = false;
    
    Serial.print("Response: ");
    while(dra_serial.available() && buffer_index < 99) {
        char c = dra_serial.read();
        response_buffer[buffer_index++] = c;
        
        if (c >= 32 && c <= 126) {
            Serial.write(c);
        } else if (c == '\r') {
            Serial.print("<CR>");
        } else if (c == '\n') {
            Serial.print("<LF>");
        } else {
            Serial.printf("[0x%02X]", c);
        }
    }
    response_buffer[buffer_index] = '\0';
    
    // 檢查是否包含正確的回應
    if (strstr(response_buffer, "+DMOCONNECT:0") != NULL) {
        Serial.println(" *** SA818 CONNECTION SUCCESS! ***");
        found_dmoconnect = true;
    } else if (strstr(response_buffer, "+DMOCONNECT") != NULL) {
        Serial.println(" *** PARTIAL SA818 RESPONSE FOUND ***");
    } else if (strstr(response_buffer, "DMOCONNECT") != NULL) {
        Serial.println(" *** DMOCONNECT TEXT DETECTED ***");
    } else {
        Serial.printf(" *** NO RESPONSE (%d bytes) ***\n", buffer_index);
    }
    
    Serial.println("SA818_Init: Done...");

    if (!dra) {
        Serial.println("\nError while configuring DRA818");
    }

    // 嘗試握手 (固定使用 9600 baud rate)
    Serial.println("Attempting handshake with SA818 at 9600 bps...");
    
    int handshake_attempts = 3;
    bool handshake_success = false;
    
    for(int attempt = 1; attempt <= handshake_attempts; attempt++) {
        Serial.printf("Handshake attempt %d/%d at 9600 bps\n", attempt, handshake_attempts);
        
        // 清空緩衝區
        while(dra_serial.available()) {
            dra_serial.read();
        }
        
        if (dra->handshake() == true) {
            Serial.println("*** SA818 HANDSHAKE SUCCESSFUL! ***");
            handshake_success = true;
            break;
        } else {
            Serial.printf("Handshake attempt %d failed\n", attempt);
            delay(1000);
        }
    }
    
    if (!handshake_success) {
        Serial.println("SA818 handshake failed at 9600 bps.");
        Serial.println("Check SA818 connections, power supply, and baud rate setting.");
        Serial.println("Continuing with initialization (some features may not work)...");
    }
    
    #if DRA818_CONFIG_UHF
        dra->group(DRA818_12K5, DRA818_UHF_MIN, DRA818_UHF_MIN, 0, 4, 0);
        dra->volume(8);
        freq = DRA818_UHF_MIN;
    #else
        dra->group(DRA818_12K5, DRA818_VHF_MIN, DRA818_VHF_MIN, 0, 4, 0);
        dra->volume(8);
        freq = DRA818_VHF_MIN;
    #endif
    Serial.println("SA818_Init: Starting ... ");
}

void HAL::SA818_scan()
{
    return;
    char buf[14];
    if (!dra) return;
    dtostrf(freq, 8, 4, buf);  // convert frequency to string with right precision
    Serial.print(String("SA818_Update: Scanning frequency ") +  String(buf) + String(" kHz ..."));
    /* scan the frequency */
    if (dra->scan(freq)) Serial.print("Found");
        Serial.println("");
    freq += 0.0125; //12.5kHz step
    #if DRA818_CONFIG_UHF
        if (freq > DRA818_UHF_MAX) freq = DRA818_UHF_MIN; // when DRA818_VHF_MAX (174.0) is reached, start over at DRA818_VHF_MIN (134.0)
    #else
        if (freq > DRA818_VHF_MAX) freq = DRA818_VHF_MIN; // when DRA818_VHF_MAX (174.0) is reached, start over at DRA818_VHF_MIN (134.0)
    #endif
}

void HAL::SA818_GetInfo(SA818_Info_t* info)
{
    //Serial.println("SA818_GetInfo");
    memset(info, 0, sizeof(SA818_Info_t));

    // 檢查 SA818 是否已初始化並可用
    if (dra != nullptr) {
        // SA818 已初始化，返回實際配置資訊
        info->channel = 2;
        #if DRA818_CONFIG_UHF
            info->freq_rx = DRA818_UHF_MIN;  // 使用實際頻率
            info->freq_tx = DRA818_UHF_MIN;
        #else
            info->freq_rx = DRA818_VHF_MIN;  // 使用實際頻率
            info->freq_tx = DRA818_VHF_MIN;
        #endif
        info->squelch = 4;    // 實際靜噪設定
        info->volume = 8;     // 實際音量設定
        info->ctcss_rx = 0;   // 實際 CTCSS 設定
        info->ctcss_tx = 0;
        info->BW = DRA818_12K5;  // 頻寬設定
        info->SQ = 4;
    } else {
        // SA818 未初始化或初始化失敗，返回零值
        info->channel = 0;
        info->freq_rx = 0.0;
        info->freq_tx = 0.0;
        info->squelch = 0;
        info->volume = 0;
        info->ctcss_rx = 0;
        info->ctcss_tx = 0;
        info->BW = 0;
        info->SQ = 0;
    }
}

// PTT (Push To Talk) 功能實現

void HAL::PTT_Init() {
    Serial.printf("PTT_Init: Configuring PTT pin GPIO %d\n", CONFIG_SA818_PTT_PIN);
    
    // 配置 PTT 引腳為輸入模式，啟用內部上拉
    pinMode(CONFIG_SA818_PTT_PIN, INPUT_PULLUP);
    
    // 初始化狀態
    ptt_state = false;
    prev_ptt_state = false;
    ptt_initialized = true;
    
    Serial.println("PTT_Init: PTT system initialized");
}

bool HAL::PTT_IsPressed() {
    if (!ptt_initialized) {
        return false;
    }
    
    // PTT 按鍵通常是低電平觸發（按下時接地）
    bool current_state = (digitalRead(CONFIG_SA818_PTT_PIN) == LOW);
    
    // 更新狀態
    prev_ptt_state = ptt_state;
    ptt_state = current_state;
    
    return ptt_state;
}

void HAL::PTT_SetTransmit(bool enable) {
    if (!dra) {
        Serial.println("PTT_SetTransmit: SA818 not initialized");
        return;
    }
    
    if (enable) {
        Serial.println("PTT: Enabling transmission mode");
        // 這裡可以添加 SA818 發送模式的設定
        // 例如：dra->transmit(true);
        // 注意：實際的 SA818 傳送控制可能需要根據庫的 API 調整
    } else {
        Serial.println("PTT: Disabling transmission mode (receive mode)");
        // 這裡可以添加 SA818 接收模式的設定
        // 例如：dra->transmit(false);
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
    if (!dra) {
        Serial.println("SA818_UpdateGroup: SA818 not initialized");
        return false;
    }
    
    float frequency = getSA818Frequency(current_power_mode, current_channel);
    if (frequency == 0.0) {
        Serial.printf("SA818_UpdateGroup: Failed to get frequency\n");
        return false;
    }
    
    // group(bandwidth, freq_tx, freq_rx, ctcss_tx, squelch, ctcss_rx)
    Serial.printf("SA818_UpdateGroup: freq=%.4f, CTCSS=%d, SQ=%d\n", 
                  frequency, g_sa818_ctcss, g_sa818_squelch);
    
    int result = dra->group(DRA818_12K5, frequency, frequency, 
                            g_sa818_ctcss, g_sa818_squelch, g_sa818_ctcss);
    
    if (result == 1) {
        Serial.println("SA818_UpdateGroup: Success");
        return true;
    } else {
        Serial.println("SA818_UpdateGroup: Failed");
        return false;
    }
}

bool HAL::SA818_SetChannel(int channel, SA818_PowerMode powerMode) {
    if (!dra) {
        Serial.println("SA818_SetChannel: SA818 not initialized");
        return false;
    }
    
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
    int result = dra->group(DRA818_12K5, frequency, frequency, 
                            g_sa818_ctcss, g_sa818_squelch, g_sa818_ctcss);
    
    if (result == 1) {
        current_channel = channel;
        current_power_mode = powerMode;
        Serial.printf("SA818_SetChannel: Successfully set to channel %d (%s, %.4f MHz)\n", 
                      channel, getPowerModeName(powerMode), frequency);
        return true;
    } else {
        Serial.printf("SA818_SetChannel: Failed to set channel %d\n", channel);
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
    if (!dra) return;
    if (vol < 1) vol = 1;
    if (vol > 8) vol = 8;
    g_sa818_volume = vol;
    dra->volume(vol); // 實際設定到 SA818
    Serial.printf("HAL_SA818: Set volume to %d\n", vol);
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