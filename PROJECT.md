# ProPrj_PLT251129004125-esp32_WT02

ESP32 + SA818 雙向手持對講機 + GPS 追蹤器（LVGL UI）

## 硬體平台

| 元件 | 型號/備註 |
|------|-----------|
| 主控 | ESP32（ESP32-Dev board） |
| 螢幕 | SSD1306 128×64 OLED（I2C） |
| 對講機模組 | SA818 VHF FM Transceiver |
| 感測器 | IMU（慣性測量）、MAG（磁力計）、GPS |
| 音訊 | 蜂鳴器 Buzzer、Audio PA |
| 按鍵 | 3 鍵：MENU/OK(GPIO32)、UP/BACK(GPIO33)、DOWN/FN(GPIO34) |
| 電源 | 鋰電池 + 充電管理 |

## 軟體架構

```
main.cpp
  └─ setup()
       ├─ HAL::HAL_Init()          # 底層硬體抽象層初始化
       ├─ Port_Init()               # LVGL 顯示Port初始化
       ├─ App_Init()               # 應用層初始化
       └─ TEST::Test_Init()        # 單元測試（USE_TEST編譯旗標）
  └─ loop()
       ├─ TEST::Test_App()         # 單元測試（USE_TEST）
       ├─ wifiManager.loop()        # WiFi Manager（ENABLE_AUTO_OTA_CHECK）
       ├─ otaUpdater.handleAutoCheck()
       └─ HAL::HAL_Update()        # 硬體狀態更新 + LVGL task tick
```

### HAL（硬體抽象層）

| 模組 | 功能 |
|------|------|
| **I2C** | 掃描、讀寫、SCL/SDA pin 控制 |
| **IMU** | 慣性測量單元（加速度/陀螺儀） |
| **MAG** | 磁力計（電子羅盤） |
| **GPS** | NMEA 解析、距離計算 |
| **Power** | 電池電壓/百分比、充電狀態 |
| **Clock** | RTC 時間、NTP 同步 |
| **Button** | 3 按鍵事件（MENU/UP/DOWN） |
| **WiFi** | STA/AP 模式切換 |
| **BLE** | 藍牙 HID（鍵盤/滑鼠） |
| **OTA** | 遠端韌體更新 |
| **SA818** | 對講機模組控制（頻道、功率、CTCSS、靜噪） |
| **MicroAPRS** | APRS-ESP 追蹤模組 |
| **U8g2** | SSD1306 OLED 顯示（不經 LVGL） |
| **Backlight** | 螢幕背光漸亮/漸暗 |

### 應用層（App）

```
App_Init()
  ├─ lv_group_create()                    # LVGL 輸入群組
  ├─ DataProc_Init()                       # 資料處理節點系統
  ├─ ACCOUNT_SEND_CMD(SysConfig, LOAD)     # 載入系統設定
  ├─ lv_obj_set_style_bg_color(black)     # OLED 背景黑色
  ├─ StatusBar_Create(lv_layer_top())     # 狀態列（最上層）
  └─ PageManager
       ├─ Install("Startup",  ...)        # 安裝 4 個頁面
       ├─ Install("Radio",     ...)
       ├─ Install("Trekking",  ...)
       ├─ Install("System",    ...)
       ├─ Install("Status",    ...)
       ├─ SetGlobalLoadAnimType(OVER_TOP, 500ms)
       └─ Push("Pages/Startup")             # 預設起始頁
```

### 頁面（Pages）

| 頁面 |職責 |
|------|------|
| **Startup** | 開機 splash 動畫，结束後自動跳轉 |
| **Radio** | 對講機介面（頻道、功率、CTCSS、靜噪、PTT） |
| **Trekking** | GPS 追蹤模式（地點標記、軌跡記錄） |
| **System** | 系統設定（WiFi、OTA、版本資訊） |
| **Status** | 即時狀態（電量、訊號、GPS 位置） |

### 框架與 Library

| Library | 版本 | 用途 |
|---------|------|------|
| **LVGL** | ^8.4.0 | 嵌入式 GUI 框架 |
| **U8g2** | ^2.35.9 | OLED 文字/圖形渲染 |
| **ArduinoJson** | ^7.0.4 | JSON 解析（NTP、OTA version check） |
| **ESP-Music** | ^1.0.0 | 音樂播放（？對講機音訊） |

## 編譯設定（platformio.ini）

```ini
[env:esp32dev]
platform   = espressif32
board      = esp32dev
framework  = arduino
monitor_speed = 115200

lib_deps   =
    lvgl/lvgl@^8.4.0
    olikraus/U8g2@^2.35.9
    bblanchon/ArduinoJson@^7.0.4
    ErtugrulKra/ESP-Music@^1.0.0

build_flags =
    -D BOARD_ESP32DR
    -D LV_CONF_INCLUDE_SIMPLE
    ;-D USE_TEST
    ;-D ENABLE_AUTO_OTA_CHECK
    ;-D USE_SCREEN_SSD1306
```

## 感測器/周邊定義（Pinout.h）

| GPIO | 功能 |
|------|------|
| 32 | BUTTON_MENU_OK |
| 33 | BUTTON_UP_BACK |
| 34 | BUTTON_DOWN_FN |
| I2C SDA/SCL | SSD1306、IMU、MAG |

## 關鍵設計模式

### 1. DataProc 資料處理節點
- `DataProc::Center()->AccountMain.Notify("ACT", &info, sizeof(info))`
- 各帳戶（Storage、SysConfig、Radio...）以 Observer 模式接收通知
- PingPongBuffer 雙緩衝，避免讀寫競爭

### 2. PageManager 頁面管理
- `PageManager::Install(name, path)` — 註冊頁面
- `PageManager::Push(path)` — 壓入新頁（帶動畫）
- `PageManager::Pop()` — 彈出返回
- 支援拖曳手勢（`PM_Drag.cpp`）、動畫（`PM_Anim.cpp`）

### 3. SA818 對講機控制
- 頻道切換：`SA818_SetChannel(channel, powerMode)`
- 功率模式：`SA818_SetHighLowPower(bool)`
- PTT 發射：`PTT_IsPressed()` + `PTT_SetTransmit(bool)`
- CTCSS/靜噪：類比亞音信號

### 4. OTA 遠端更新
- WiFi 連線 → 檢查 version URL → 下載新 bin → 重啟燒錄
- `wifiManager.autoConnectToWiFi()` 自動重連
- `otaUpdater.checkForUpdates()` 輪詢

## 原始碼統計

| 類型 | 數量 |
|------|------|
| 總檔案數 | 109 |
| 目錄數 | 21 |
| 主要類別 | HAL、DataProc、PageManager、WiFiManager、OTA |

## 待研究/待完成

- [ ] SD 卡功能（HAL 中 SD 模組預留）
- [ ] BLE HID 鍵盤/滑鼠功能
- [ ] MicroAPRS APRS 追蹤功能
- [ ] `USE_TEST` 模式具體行為
- [ ] Trekking/Status 頁面感測器整合（GPS、IMU、MAG、BME280）

## Git

```
Remote: github.com/JiangAlex/ProPrj_PLT251129004125-esp32_WT02
```
