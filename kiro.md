# Web GUI - SSD1306 OLED Simulator

## Overview
ESP32 Web Server 透過 WebSocket 將 U8g2 framebuffer (128x64, 1024 bytes) 即時推送到瀏覽器，Canvas 渲染模擬 OLED 畫面，並支援鍵盤按鍵注入。

## Architecture
- **Server**: ESPAsyncWebServer + AsyncWebSocket
- **Display**: u8g2.getBufferPtr() → 1024 bytes binary → WebSocket → Canvas
- **Input**: Browser keydown/keyup → WebSocket 2-byte binary → virtual button → keypad_read()
- **Compile Flag**: `-D ENABLE_WEB_GUI`
- **HTML**: PROGMEM embedded (no SPIFFS)
- **WebSocket overflow protection**: `ws.availableForWriteAll()` check before `binaryAll()`

## Keyboard Mapping
| Key | Button | LVGL |
|-----|--------|------|
| Enter | MENU/OK | LV_KEY_ENTER |
| ArrowUp | UP/BACK | LV_KEY_PREV |
| ArrowDown | DOWN/FN | LV_KEY_NEXT |

PTT: 實體按鈕 only, not mapped to Web.

## Hardware Pin Configuration
| Module | Interface | Pins |
|--------|-----------|------|
| SA818 Radio | Serial2 | RX=17, TX=16, PTT=13, PD=26, HL=18 |
| GPS | Serial1 | RX=14, TX=27, 9600 baud |
| GPS Data | `GPS_Info_t` | lat, lon, alt, speed, course(0-360°), satellites, isValid |
| OLED SSD1306 | I2C | SDA=21, SCL=22, Addr=0x3C |
| Buttons | GPIO | OK=32, UP=33, DOWN=34, PTT=35 |

## Framebuffer Format
U8g2 SSD1306 full buffer: 8 pages × 128 columns = 1024 bytes.
Each byte = 8 vertical pixels (LSB = top pixel of that page).
Page 0 = rows 0-7, Page 1 = rows 8-15, ..., Page 7 = rows 56-63.

## Data Flow
```
LVGL render → disp_flush_cb → u8g2.sendBuffer() + set dirty flag
                                    ↓
WebServer task (100ms) → if dirty & availableForWriteAll → ws.binaryAll(buffer, 1024)
                                    ↓
Browser WebSocket → ArrayBuffer → bit-unpack → Canvas 128x64 (4x scaled)

Browser keydown/keyup → ws.send([key_id, state]) → ESP32 onEvent
                                    ↓
volatile virtual_key/virtual_pressed → keypad_read() priority check
```

---

## Full GUI Redesign — Professional Hiking Radio

### Hardware
- ESP32-WROOM-32E-N16 (16MB Flash)
- SSD1306 128x64 OLED (I2C 0x3C)
- SA818 Radio Module (Serial2, GPIO 16/17, PTT=13)
- GPS Module (Serial1, GPIO 14/27, 9600 baud)
- Buttons: OK=GPIO32, UP=GPIO33, DOWN=GPIO34, PTT=GPIO35
- WebGUI: ESPAsyncWebServer + WebSocket (OLED streaming + key injection + GPX upload + OTA)

### 16MB Flash Partition (OTA + SPIFFS)
```
nvs,       data, nvs,     0x9000,    0x5000    (20KB)
otadata,   data, ota,     0xE000,    0x2000    (8KB)
ota_0,     app,  ota_0,   0x10000,   0x300000  (3MB)
ota_1,     app,  ota_1,   0x310000,  0x300000  (3MB)
spiffs,    data, spiffs,  0x610000,  0x9F0000  (~10MB)
```

### Page Structure (128x64 OLED)
```
┌─────────────────────────┐ 0
│ StatusBar (16px)        │ Time | Battery/TX | WiFi
├─────────────────────────┤ 16
│ Main Content (34px)     │ Scrollable list / Map canvas
├─────────────────────────┤ 50
│ FuncBar (14px)          │ [ACTION] [BACK]
└─────────────────────────┘ 64
```

### Pages

1. **StartUp** (Main Menu): Radio, Trekking, Map, Status, System + 底部日期
2. **Radio**: CH/CTCSS/Power/RSSI/VOL/SQ + [SCAN][BACK]
3. **Trekking**: Temp/Alt/Pres/Asc/Dist/Time/Stat/GPX/Profile + [START/PAUSE][BACK]
4. **Profile** (子頁面): 海拔剖面圖 (128x34 canvas) + [BACK]
5. **Map**: GPX track rendering + GPS position + waypoint navigation
6. **Status**: TEMP/ALT/PRES/STEP/COMP + [BACK]
7. **System**: WiFi/IP/Heap/Bright/TZ/Clock(12H/24H)/GPS/Ver/OTA/Reset WiFi/Reboot + [OK][BACK]

### Trekking Page
- 直接進入資料顯示（無 entry mode）
- 長按 OK 3 秒 = STOP（重置所有數據）
- GPS 即時追蹤：距離（>2m 過濾）、海拔、爬升（>1m 過濾）
- GPX 項目：OK 循環切換已上傳的 GPX 檔案
- Profile 項目：OK 進入海拔剖面圖頁面

### Profile Page (海拔剖面圖 — 全螢幕)
- **全螢幕 128x64** — 無 StatusBar、無 FuncBar
- 進入時顯示完整海拔剖面（auto-fit 全距離範圍）
- **操作模式**：
  - OK 短按：切換 Pan ↔ Zoom 模式
  - 右上角顯示模式圖示：`P`（平移）/ `Z`（縮放）
  - UP/DOWN（Pan 模式）：沿距離軸左右平移（20%/次）
  - UP/DOWN（Zoom 模式）：放大/縮小距離軸
  - 長按 OK 3 秒：返回 Trekking 頁面
- X 軸 = 距離(km)，Y 軸 = 海拔(m)
- 從 SPIFFS 讀取選中的 GPX（fallback 第一個檔案）
- Bresenham 折線繪製，自動 clamp 到可視範圍

### Map Page Design (全螢幕重新設計)
- **全螢幕 128x64** — 無 StatusBar、無 FuncBar
- 進入時顯示完整 GPX 軌跡（auto-fit 所有點到螢幕）
- **操作模式**：
  - OK 短按：切換 Pan ↔ Zoom 模式
  - 右上角顯示模式圖示：`P`（平移）/ `Z`（縮放）
  - UP/DOWN（Pan 模式）：沿軌跡方向前進/後退，視窗中心跟著軌跡點移動
  - UP/DOWN（Zoom 模式）：放大/縮小
  - 長按 OK 3 秒：返回主選單
- **GPS 位置**：▲ 標記
  - 在可視範圍內：正常顯示位置
  - 超出可視範圍：▲ clamp 到螢幕邊緣（指示方向）
- GPX 軌跡白色折線，航點用十字標記

### System Page Settings (NVS Persistent)
| Setting | NVS Key | Range |
|---------|---------|-------|
| Brightness | system/bright | 1-8 |
| Timezone | system/tz | UTC-12 ~ UTC+14 |
| Clock Format | system/24h | 12H / 24H |
| Radio Channel | radio/ch | 1-20 |
| Radio Power | radio/pwr | LOW/HIGH |
| Radio Volume | radio/vol | 1-8 |
| Radio Squelch | radio/sq | 0-8 |
| Radio CTCSS | radio/ctcss | 0-38 |

### OTA Update
- 開機自動檢查版本（HTTP GET /version）
- 有新版本 → OLED 顯示對話框 `>Yes / No`（GPIO polling）
- 選 Yes → 進度條畫面 (lv_bar + 百分比)
- 手動更新 only（不自動下載）
- Version 定義在 `src/App/Configs/Version.h`

### StatusBar
- 左側：時間（支援 12H/24H 格式）
- 右側：電池百分比 / PTT 按下時顯示 "TX"
- NTP 同步：Clock_Init 後自動同步（WiFi 已連線時）

### WebGUI Extensions
- GPX file upload (HTTP POST /upload?name=xxx → parse → /gpx/NNN.bin)
- GPX file list (HTTP GET /gpxlist)
- GPX clear all (HTTP GET /gpxclear)
- OTA firmware upload (HTTP POST /ota → Update library)
- Max 500 track points + 50 waypoints per GPX
- 檔案大小限制 100KB（大檔案用 server.py 精簡）

### Multi-GPX File Storage
```
/gpx/index.txt    — 每行一個序號 (1, 2, 3...)
/gpx/001.bin      — 二進制格式: [uint16 trackCount][uint16 wptCount][TrackPoint...][Waypoint...]
/gpx/002.bin
...
```
- TrackPoint: `{float lat, float lon, float ele}` (12 bytes)
- Waypoint: `{float lat, float lon, char name[12]}` (20 bytes)
- 上傳自動分配下一個序號
- Trekking 頁面 GPX 項目循環切換
- Profile 和 Map 頁面讀取當前選中的 GPX

### web/server.py (電腦端)
- OTA firmware server (port 8080)
- GPX 精簡 + 轉發：POST /gpx?name=xxx
  - 接收大 GPX 檔案（支援 MB 級）
  - XML 解析，均勻取樣 500 個點
  - 保留 lat/lon/ele
  - 轉發精簡後的 GPX 到 ESP32 /upload endpoint
- 使用：`curl -X POST "http://localhost:8080/gpx?name=file.gpx" --data-binary @/path/to/file.gpx`

### Navigation Logic (all pages)
- UP/DOWN: navigate menu items
- DOWN past last item → enter FuncBar
- UP in FuncBar → back to menu
- OK: execute selected item or FuncBar action
- `>` = selected, `*` = edit mode
- 進入頁面時 debounce（lastBtnTime = millis()）防止 OK 誤觸

### LVGL Race Condition Fix
- `main.cpp`: App_Init() 包在 `xSemaphoreTake/Give(xGuiSemaphore)` 中
- 防止 LVGL task 在 UI 初始化完成前渲染導致 LoadProhibited crash

---

## Notes

- Map page 使用 LV_IMG_CF_ALPHA_1BIT canvas，適合單色 OLED
- GPX upload 使用 String 累加，限制 100KB。大檔案用 server.py 精簡
- SA818 setGroup 失敗時仍更新 internal state（UI 反映使用者意圖）
- Schematic reference: `Data/SCH_Schematic1_2_2026-02-23.pdf`

## Completed Tasks
- [x] Fix LVGL race condition crash (semaphore guard)
- [x] OTA dialog (Yes/No) + progress bar on OLED
- [x] NTP sync after Clock_Init
- [x] NVS persistence (Radio + System settings)
- [x] System page: Timezone, Clock 12H/24H
- [x] StartUp: real date from Clock
- [x] GPS driver verified (Serial1 RX=14 TX=27)
- [x] Trekking: GPS integration (distance, altitude, ascent)
- [x] Trekking: remove entry mode, add long-press STOP
- [x] Trekking: Profile page (elevation chart)
- [x] Trekking: multi-GPX file selection
- [x] WebSocket overflow protection
- [x] WebGUI: GPX list + clear all
- [x] StatusBar: TX indicator when PTT pressed
- [x] Map: auto-load first GPX if no selection
- [x] UART pin conflict resolved (SA818=Serial2, GPS=Serial1)
- [x] GPS course/heading field added to GPS_Info_t
- [x] Profile: fullscreen Pan/Zoom, GPS position ▽ marker, distance scale ticks
- [x] Map: fullscreen Pan/Zoom, hide StatusBar

## Future Features (TODO)

- **APRS** — SA818 發送/接收 APRS 封包（位置報告、緊急求救）
- **BLE 連接手機** — 手機 APP 傳送航點/接收 GPS 資料
- **A2DP** — Bluetooth audio streaming
- **軌跡記錄** — 自動記錄 GPS 軌跡到 SPIFFS，事後下載
- **電池電量校準** — ADC 讀取電池電壓，StatusBar 顯示百分比
- **緊急求救** — 長按 PTT 發送預設 APRS 緊急訊息
- **多語言** — 中/英切換
- **WebGUI 即時地圖** — 瀏覽器端用 Leaflet 顯示 GPS 位置 + GPX 軌跡
- **Status 頁面** — 接入真實感測器（BMP280 溫度/氣壓）
