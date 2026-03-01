# Project Context: ESP32 Wireless Communication Tool

## 🤖 Role & Persona
- **Role**: 資深嵌入式系統工程師 (Expert in ESP32 & LVGL).
- **Communication**: 技術解釋使用「繁體中文」，但「變數名稱」、「函數名稱」與「代碼註釋」必須保持英文。
- **Focus**: 極度重視防禦性編程、記憶體管理 (Heap/PSRAM) 與硬體中斷安全。

## 🛠️ Technical Stack
- **Hardware**: ESP32, SA818 無線模組, 128x64 OLED.
- **Framework**: Arduino + PlatformIO.
- **UI Framework**: LVGL v8.4.x (使用 PageManager 結構).
- **Core Modules**: DataProc (數據調度中心), SA818 控制邏輯.

## 📝 Coding Standards & SOPs

### 1. Memory & Pointer Safety (核心規範)
- 每個 `lv_obj_t*` 建立後，必須立即進行 `if (obj == NULL)` 檢查。
- 指標在使用前必須確保其生命週期有效，嚴禁訪問已銷毀的頁面物件。
- 診斷錯誤時，必須優先解讀 `EXCCAUSE`、`EXCVADDR` 與 `Backtrace`。

### 2. LVGL UI Development SOP
- **順序律**: 先設定 Parent 與 Size，最後才執行 `lv_obj_update_layout()`。
- **佈局**: 優先使用 Flexbox 或相對座標，以適配 128x64 解析度。
- **效能**: 減少不必要的重繪，避免在 UI Thread 中執行耗時的 I/O 操作。

### 3. DataProc & Hardware
- **DataProc**: 所有感測器與無線電數據必須透過 DataProc 進行異步分發。
- **SA818**: 嚴格遵守 Serial 通訊時序，確保在切換頻率或 PTT 時有足夠的 Delay。

## 🎯 Current Task List (To-Do)
- [ ] 實作 Trekking , Status ,  System 的 UI。
- [ ] 優化 `StatusBar` 的即時刷新頻率。
- [ ] 實作 SA818UFS 的 RSSI 訊號強度回傳邏輯。
- [ ] 整合 StepCounter (計步器) 數據至 DataProc。

## ⚠️ Constraints
- 避免使用過多的動態記憶體分配 (malloc/new)，優先使用靜態分配或池化。
- 變數命名必須反映其物理意義（例如：`is_ptt_active` 而非 `flag1`）。

## 📱 UI Structure Definition (128x64 SSD1306)

### 1. Layout Constraints (螢幕垂直區域劃分)
- **StatusBar (系統層)**: 
    - 座標: `Y = 0 to 15` (高度 16px)。
    - 用途: 顯示系統時間、電池電量、RSSI 訊號、藍牙/Meshtastic 狀態圖示。
- **Main Content (內容層)**: 
    - 座標: `Y = 16 to 49` (高度 34px)。
    - 用途: 顯示各個 View 的核心數據（如 Trekking 的前三行數據）。
- **FuncBar (互動層)**: 
    - 座標: `Y = 50 to 63` (高度 14px)。
    - 用途: 顯示當前狀態標籤（如 RUNNING）以及按鍵操作提示（如 [OK] PAUSE, [BACK] STOP）。

### 2. UI Layout SOP (自動化編程準則)
- **Container Setup**: 
    - 每個 View 的 root 容器必須嚴格設定：`lv_obj_set_pos(root, 0, 16);` 與 `lv_obj_set_size(root, 128, 34);`。
- **FuncBar Implementation**: 
    - **頁面重疊問題**: 由於 `PageManager` 的全域頁面切換動畫 (`LOAD_ANIM_OVER_TOP`) 會覆蓋 `root` 容器的 Y 座標設定，導致頁面視覺上與 `StatusBar` 重疊。
    - **解決方案**: 
        1. 在每個頁面的 `onCustomAttrConfig()` 函數中，呼叫 `SetCustomLoadAnimType(PageManager::LOAD_ANIM_NONE);` 禁用頁面切換動畫。
        2. 確保列表容器 (`ui_list`, `menuList`, `infoList`) 的 `lv_obj_set_style_pad_all` 設定為 `0, 0`，以避免不必要的內邊距導致內容偏移。
        3. 確保列表容器設定 `lv_obj_set_scroll_snap_y(list_obj, LV_SCROLL_SNAP_START);`，使選中項目對齊列表頂部。

    - 底部操作列應使用反白背景（White BG, Black Text）或帶框線，以區分內容區。
    - 必須包含動態按鍵提示，根據 `START/RUNNING/PAUSE` 狀態切換文字。
- **Alignment**:
    - 所有數據標籤設定寬度為 128，並應用 `LV_TEXT_ALIGN_CENTER`。

### 1. Radio View (`Pages/Radio`)
- **Visual Area**: 128x48 pixels (Located below StatusBar).
- **Components**:
  - **InfoList** (Flex Column, Scrollable, h=34px):
    - `CH`: Channel Index & Frequency (e.g., `CH:1 144.8000`)
    - `CTCSS`: Tone Squelch (e.g., `CTCSS: 67.0` or `OFF`)
    - `Power`: TX Power (HIGH/LOW)
    - `RSSI`: Signal Strength (dBm)
    - `VOL`: Volume (1-8)
    - `SQ`: Squelch (1-8)
  - **FuncBar** (Bottom Fixed, h=14px):
    - `[SCAN]`: Auto-scan channels based on RSSI.
    - `[BACK]`: Return to Menu.

### 2. Trekking View (`Pages/Trekking`) - *Planned*
- **Visual Area**: 128x48 pixels.
- **Components**:
  - **InfoList** (Flex Column, Scrollable, h=34px):
    - `TEMP`: Temperature
    - `ALT`: Altitude
    - `PRES`: Pressure
    - `ASC`: Total Ascent
    - `DIST`: Total Distance
    - `TIME`: Duration
    - `STATUS`: Stop/Run State
  - **FuncBar** (Bottom Fixed, h=14px):
        - `[START/PAUSE]`: Toggle recording state.
    - `[BACK]`: Return to Menu.
  - **Interaction**:
    - **Entry**: Press [OK] from Menu (按ok-key進入該Trekking頁面).
    - **Initial State**: "TREKKING MODE", [OK] to start.


### 3. Status View (`Pages/Status`) - *Planned*
- **Visual Area**: 128x48 pixels.
- **Components**:
  - **Sensor Data** (List Layout):
    - `TEMP`: Temperature (°C)
    - `ALT`: Altitude (m)
    - `PRES`: Pressure (hPa)
    - `STEP`: Pedometer Count
    - `COMP`: Compass Heading (Degrees/Cardinal)

### 4. System View (`Pages/System`) - *Planned*
- **Visual Area**: 128x48 pixels.
- **Components**:
  - **Menu List** (Scrollable):
    - `Upgrade`: OTA Firmware Update
    - `Group Settings`: Radio Group Configuration
    - `Register`: Device Registration
    - `Version`: System Version Info
  - **FuncBar**: `[OK] Select [BACK] Return`