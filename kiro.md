# Web GUI - SSD1306 OLED Simulator

## Overview
ESP32 Web Server 透過 WebSocket 將 U8g2 framebuffer (128x64, 1024 bytes) 即時推送到瀏覽器，Canvas 渲染模擬 OLED 畫面，並支援鍵盤按鍵注入。

## Architecture
- **Server**: ESPAsyncWebServer + AsyncWebSocket
- **Display**: u8g2.getBufferPtr() → 1024 bytes binary → WebSocket → Canvas
- **Input**: Browser keydown/keyup → WebSocket 2-byte binary → virtual button → keypad_read()
- **Compile Flag**: `-D ENABLE_WEB_GUI`
- **HTML**: PROGMEM embedded (no SPIFFS)

## Keyboard Mapping
| Key | Button | LVGL |
|-----|--------|------|
| Enter | MENU/OK | LV_KEY_ENTER |
| ArrowUp | UP/BACK | LV_KEY_PREV |
| ArrowDown | DOWN/FN | LV_KEY_NEXT |

PTT: 實體按鈕 only, not mapped to Web.

## Files Modified/Created
- `platformio.ini` — add ESPAsyncWebServer, AsyncTCP deps + ENABLE_WEB_GUI flag
- `src/App/Common/HAL/HAL.h` — add WebServer_Init/Update declarations
- `src/App/Common/HAL/HAL.cpp` — call WebServer_Init in HAL_Init
- `src/App/Common/HAL/HAL_WebServer.cpp` — NEW: web server + websocket + HTML
- `src/App/Common/Port/lv_port/lv_port_disp.cpp` — set dirty flag on flush
- `src/App/Common/Port/lv_port/lv_port_indev.cpp` — virtual button injection

## Framebuffer Format
U8g2 SSD1306 full buffer: 8 pages × 128 columns = 1024 bytes.
Each byte = 8 vertical pixels (LSB = top pixel of that page).
Page 0 = rows 0-7, Page 1 = rows 8-15, ..., Page 7 = rows 56-63.

## Data Flow
```
LVGL render → disp_flush_cb → u8g2.sendBuffer() + set dirty flag
                                    ↓
WebServer task (100ms) → if dirty → ws.binaryAll(buffer, 1024)
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
- SA818 Radio Module (UART1, GPIO 13/14)
- GPS Module (UART2, GPIO 16 RX / GPIO 17 TX, 9600 baud)
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
│ StatusBar (16px)        │ Time | Battery | WiFi | PTT
├─────────────────────────┤ 16
│ Main Content (34px)     │ Scrollable list / Map canvas
├─────────────────────────┤ 50
│ FuncBar (14px)          │ [ACTION] [BACK]
└─────────────────────────┘ 64
```

### Pages

1. **StartUp** (Main Menu): Radio, Trekking, Map, Status, System
2. **Radio** (keep existing): CH/CTCSS/Power/RSSI/VOL/SQ + [SCAN][BACK]
3. **Trekking** (keep existing): Temp/Alt/Pres/Asc/Dist/Time/Stat + [START][BACK]
4. **Map** (NEW): GPX track rendering + GPS position + waypoint navigation
5. **Status** (keep existing): TEMP/ALT/PRES/STEP/COMP + [BACK]
6. **System** (rewrite): WiFi/IP/Heap/Brightness/GPS sat/Version/OTA Update/Reset WiFi/Reboot + [OK][BACK]

### Map Page Design
- 128x34 pixel canvas for track rendering
- GPS position shown as ▲ with heading
- GPX track drawn as pixel lines (auto-fit bounding box)
- Waypoints shown as ★
- FuncBar: `[WPT] [BACK]`
- Bottom info: waypoint name + distance + bearing
- UP/DOWN to switch target waypoint

### WebGUI Extensions
- GPX file upload (HTTP POST /upload → parse → SPIFFS)
- OTA firmware upload (HTTP POST /ota → Update library)
- Max 500 track points + 50 waypoints per GPX

### Navigation Logic (all pages)
- UP/DOWN: navigate menu items
- DOWN past last item → enter FuncBar
- UP in FuncBar → back to menu
- OK: execute selected item or FuncBar action
- `>` = selected, `*` = edit mode

### Task List
1. Fix WebGUI virtual button for all pages (add Page.h include)
2. Partition setup (16MB OTA + SPIFFS) + platformio.ini config
3. GPS driver (TinyGPSPlus, UART2 GPIO16/17)
4. System page rewrite (WiFi/IP/Heap/Bright/GPS/Version/OTA/Reset/Reboot)
5. Map page — GPS position display
6. Map page — GPX track rendering
7. Map page — Waypoint navigation
8. WebGUI — GPX upload endpoint + parser
9. StartUp menu update (add Map)
10. WebGUI — OTA firmware upload
11. Integration test + cleanup
