#include "HAL.h"


// --- U8g2 實例的定義 ---
// 參數範例：U8G2_SSD1306_128X64_NONAME_F_HW_I2C(rotation, reset, sda, scl)
// 這裡使用 ESP32 預設的 I2C 腳位，如果沒有特別指定，使用 U8X8_PIN_NONE
// U8X8_PIN_NONE 表示使用 Wire 庫的預設/已設定腳位
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void HAL::U8g2_Init()
{
    Serial.println("HAL: Initializing U8g2 and SSD1306 display...");

    // 1. 啟動 U8g2 庫
    u8g2.begin();

    // 2. 清除顯示，確保沒有隨機像素
    u8g2.clearDisplay();

    // 3. (可選) 設置顯示模式：R0 = 無旋轉，R1 = 順時針 90 度等
    // u8g2.setFlipMode(0); // 0 = 正常方向

    // 4. (可選) 設置字體，準備好測試顯示
    // u8g2.setFont(u8g2_font_ncenB10_tr);
    // u8g2.drawStr(0, 15, "LVGL Init...");
    // u8g2.sendBuffer();

    Serial.println("HAL: SSD1306 ready.");
}

/*
void HAL::U8g2_Test()
void HAL::U8g2_Clear()
void HAL::U8g2_SetBrightness(uint8_t brightness)
void HAL::U8g2_Sleep()
void HAL::Display_Wakeup()
void HAL::U8g2_GetInfo(::Display_Info_t *info)
void HAL::U8g2_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
void HAL::U8g2_SendPixels(const uint16_t *pixels, uint32_t len)
// Global callback function pointer
void HAL::Display_SetSendFinishCallback(void (*func)(void))
// Get display instance for LVGL integration
LGFX* HAL_GetDisplayInstance()
*/

/* 執行 U8g2 測試，顯示測試圖案 */
void HAL::U8g2_Test()
{
    Serial.println("HAL: U8g2 Test - Displaying test pattern...");

    // 清除顯示
    u8g2.clearDisplay();

    // 設置字體
    u8g2.setFont(u8g2_font_ncenB10_tr);

    // 顯示測試文字
    u8g2.drawStr(0, 15, "U8g2 Test!");

    // 繪製一些圖形
    u8g2.drawBox(0, 20, 50, 30); // 繪製一個矩形框
    u8g2.drawCircle(80, 40, 15, U8G2_DRAW_ALL); // 繪製一個圓形

    // 發送緩衝區到顯示器
    u8g2.sendBuffer();

    Serial.println("HAL: U8g2 Test complete.");
}

/* 清除顯示內容 */
void HAL::U8g2_Clear()
{
    u8g2.clearDisplay();
    u8g2.sendBuffer();
}

/* brightness 範圍 0-255 */
void HAL::U8g2_SetBrightness(uint8_t brightness)
{
    // SSD1306 的對比度範圍是 0x00 到 0xFF
    u8g2.setContrast(brightness);
}
/* 進入省電模式 */
void HAL::U8g2_Sleep()
{
    u8g2.setPowerSave(1);
}
/* 退出省電模式 */
void HAL::U8g2_Wakeup()
{
    u8g2.setPowerSave(0);
}

/* 設置繪圖區域 */
/*
void HAL::U8g2_SetAddrWindow(int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
    uint32_t w = (x1 - x0 + 1);
    uint32_t h = (y1 - y0 + 1);
    u8g2.setDrawArea(x0, y0, w, h);
}
*/

/* send pixels to U8g2 display
   pixels 為 16 位元 RGB565 格式陣列
   len 為像素數量
*/
void HAL::U8g2_SendPixels(const uint16_t *pixels, uint32_t len)
{
    // SSD1306 是單色顯示器，像素資料需要轉換為位圖格式
    // 這裡假設 pixels 是 16 位元 RGB565 格式，需要轉換為單色位圖
    // 這是一個簡單的範例，實際應用中可能需要更複雜的轉換邏輯

    // 清除顯示緩衝區
    u8g2.clearBuffer();

    for (uint32_t i = 0; i < len; i++)
    {
        uint16_t pixel = pixels[i];
        // 將 RGB565 轉換為單色（閾值法）
        uint8_t r = (pixel >> 11) & 0x1F;
        uint8_t g = (pixel >> 5) & 0x3F;
        uint8_t b = pixel & 0x1F;
        uint8_t gray = (r * 299 + g * 587 + b * 114) / 1000; // 簡單灰階轉換
        if (gray > 16) // 閾值設定
        {
            u8g2.drawPixel(i % u8g2.getWidth(), i / u8g2.getWidth());
        }
    }

    // 發送緩衝區到顯示器
    u8g2.sendBuffer();
}