---
name: ssd1306-display
description: |
  SSD1306 OLED 顯示器 (128x64) 開發。使用 LVGL v8.4.x 框架。
  觸發時機：ESP32 專案需要 SSD1306/LVGL 顯示功能，包括頁面佈局、選單、文字處理等。
---

# SSD1306 Display (ESP32-WT02)

使用 **LVGL v8.4.x**，I2C 地址 **0x3C**，GPIO 4=SDA, GPIO 5=SCL。

---

## Layout 架構 (128x64)

```
┌────────────────────────────┐  ← y=0-16 (16px) StatusBar
│ 12:00            85%  📡  │
├────────────────────────────┤  ← y=16
│ [選項1]                    │  ← y=16-50 (34px) 選單區
│ [選項2]                    │
│ [選項3]                    │
├────────────────────────────┤  ← y=50
│ [確認] [▼▲] [返回]         │  ← y=50-64 (14px) 功能列
└────────────────────────────┘
```

| 區域 | Y 範圍 | 高度 |
|------|--------|------|
| StatusBar | 0-16 | 16px |
| 選單區 | 16-50 | 34px |
| 功能列 | 50-64 | 14px |

---

## 頁面範例

### Startup
```
┌────────────────────────────┐
│          WT02             │
│         [LOGO]            │
│       System Ready        │
│  ████████████░░░░░░  60%   │
└────────────────────────────┘
```

### Radio
```
┌────────────────────────────┐
│ 12:00            85%  📡  │
├────────────────────────────┤
│ CH1      CTCSS: 67.0Hz    │
│ PWR:Hi   RSSI: ████░ -80 │
│ VOL:5    CQL: OFF         │
├────────────────────────────┤
│ [Scan] [CH+] [CH-] [Back] │
└────────────────────────────┘
```

### Settings
```
┌────────────────────────────┐
│ 12:00            85%  📡  │
├────────────────────────────┤
│ Settings                  │
│ ───────────────────────── │
│ > Sleep Timeout           │
│   Backlight: 80%          │
│   Volume: 5               │
├────────────────────────────┤
│ [OK] [▼▲] [Back]         │
└────────────────────────────┘
```

---

## 字體

| 字體 | 大小 |
|------|------|
| UNSCII_8 | 8px |
| UNSCII_16 | 16px |

```cpp
lv_obj_set_style_text_font(label, &lv_font_montserrat_8, 0);
```

---

## 文字處理

### 自動換行
```cpp
lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
```

### 自動滾動
```cpp
lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
lv_obj_scroll_by(0, 10, LV_ANIM_ON);  // 手动滚动
```

### 多行
```cpp
lv_label_set_text(label, "Line1\nLine2\nLine3");
lv_obj_update_layout(label);
```

---

## 初始化

```cpp
// lv_conf.h
#define LV_HOR_RES_MAX 128
#define LV_VER_RES_MAX 64

void setup() {
  lv_init();
  // SSD1306 I2C 初始化
}
```

---

## 常見問題

- **螢幕不亮**：檢查 I2C 地址 0x3C、SCL/SDA 接線
- **刷新不完整**：調用 `lv_task_handler()`
- **記憶體不足**：減少頁面項目、使用静态分配
