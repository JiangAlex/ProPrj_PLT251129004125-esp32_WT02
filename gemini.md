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
- **導航**: 列表頁面需隱藏滾動條 (`LV_SCROLLBAR_MODE_OFF`) 但保持滾動功能，並使用 `> ` 符號指示當前選中項。

### 3. DataProc & Hardware
- **DataProc**: 所有感測器與無線電數據必須透過 DataProc 進行異步分發。
- **SA818**: 嚴格遵守 Serial 通訊時序，確保在切換頻率或 PTT 時有足夠的 Delay。

## 🎯 Current Task List (To-Do)
- [ ] 修復 `StartupView` 的 LoadProhibited 崩潰 (原因：父物件為 NULL)。
- [ ] 優化 `StatusBar` 的即時刷新頻率。
- [ ] 實作 SA818UFS 的 RSSI 訊號強度回傳邏輯。
- [ ] 整合 StepCounter (計步器) 數據至 DataProc。

## ⚠️ Constraints
- 避免使用過多的動態記憶體分配 (malloc/new)，優先使用靜態分配或池化。
- 變數命名必須反映其物理意義（例如：`is_ptt_active` 而非 `flag1`）。