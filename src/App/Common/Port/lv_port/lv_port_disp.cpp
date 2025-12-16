#include "App/Common/Port/Display.h"
#include "App/Common/HAL/HAL.h"

#define DISP_HOR_RES         CONFIG_SCREEN_HOR_RES
#define DISP_VER_RES         CONFIG_SCREEN_VER_RES
//#define DISP_BUF_SIZE        CONFIG_SCREEN_BUFFER_SIZE
#define DISP_BUF_SIZE        (DISP_HOR_RES * DISP_VER_RES / 8)

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf_1[DISP_BUF_SIZE]; // 1-bit 模式下 lv_color_t 佔用 1 byte，但實際只用 1 bit

static void disp_flush_cb(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
    // 獲取 LVGL 請求更新的矩形區域尺寸
    uint16_t w = (area->x2 - area->x1 + 1);
    uint16_t h = (area->y2 - area->y1 + 1);

    // 逐像素绘制到U8g2
    for (int16_t y = 0; y < h; y++) {
        for (int16_t x = 0; x < w; x++) {
            uint32_t idx = y * w + x;
            if (color_p[idx].full) {  // 白色像素
                u8g2.setDrawColor(1);
            } else {  // 黑色像素
                u8g2.setDrawColor(0);
            }
            u8g2.drawPixel(area->x1 + x, area->y1 + y);
        }
    }

    // 2. 將更新後的內部緩衝區發送到 SSD1306 晶片
    u8g2.sendBuffer();

    // 3. 通知 LVGL 刷新已完成，這樣 LVGL 就可以開始繪製下一個畫面
    lv_disp_flush_ready(disp);
}

void lv_port_disp_init()
{
    // 1. 初始化 LVGL 繪圖緩衝區
    // 由於 SSD1306 內存有限，我們使用單緩衝區 (buf_2 設為 NULL)
    lv_disp_draw_buf_init(&disp_buf, buf_1, nullptr, DISP_BUF_SIZE);

    // 2. 註冊 LVGL 顯示驅動
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    // 3. 設定解析度和緩衝區
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    disp_drv.draw_buf = &disp_buf;

    // 4. 設定核心回調函數
    disp_drv.flush_cb = disp_flush_cb; 
    //disp_drv.wait_cb = disp_wait_cb; 

    // 5. 註冊驅動
    lv_disp_drv_register(&disp_drv);
}