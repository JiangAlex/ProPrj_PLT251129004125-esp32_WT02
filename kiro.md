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
