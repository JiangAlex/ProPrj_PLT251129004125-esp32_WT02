#include "App.h"
#include "App/Common/HAL/HAL.h"
#include "App/Common/DataProc/DataProc.h"
#include "App/Pages/AppFactory.h"
#include "App/Pages/StatusBar/StatusBar.h"
#include "App/Utils/PageManager/PageManager.h"
#include "App/Utils/OTA/ota_updater.h"

using namespace Page;
    
#define ACCOUNT_SEND_CMD(ACT, CMD)                                         \
    do                                                                     \
    {                                                                      \
        DataProc::ACT##_Info_t info;                                       \
        DATA_PROC_INIT_STRUCT(info);                                       \
        info.cmd = DataProc::CMD;                                          \
        DataProc::Center()->AccountMain.Notify(#ACT, &info, sizeof(info)); \
    } while (0)

void App_Init()
{
    static AppFactory factory;
    static PageManager manager(&factory);

    /* Initialize the data processing node */
    Serial.print("Free heap before DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    DataProc_Init();
    Serial.print("Free heap after DataProc init: ");
    Serial.println(ESP.getFreeHeap());

    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_LOAD);

    /* 設置螢幕背景為黑色（單色 OLED 必須） */
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
    
    /* 設置 lv_layer_top 為透明 */
    lv_obj_set_style_bg_opa(lv_layer_top(), LV_OPA_TRANSP, 0);

    /* Initialize status bar - 放在 lv_layer_top 確保始終顯示在最上層 */
    Page::StatusBar_Create(lv_layer_top());

    /* Initialize pages first */
    manager.Install("Startup", "Pages/Startup");
    manager.Install("Radio",   "Pages/Radio");
    manager.Install("Trekking","Pages/Trekking");
    manager.Install("Map",     "Pages/Map");
    manager.Install("System", "Pages/System");
    manager.Install("Status",  "Pages/Status");
    manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP, 500);
    manager.Push("Pages/Startup");
    
    /* Check if OTA update was detected during WiFi init */
    if (HAL::OTA_IsUpdateAvailable()) {
        String remoteVer = otaUpdater.getRemoteVersion();
        Serial.printf("[OTA] Showing update dialog: v%s\n", remoteVer.c_str());

        // Build dialog on lv_layer_top
        lv_obj_t *overlay = lv_obj_create(lv_layer_top());
        lv_obj_remove_style_all(overlay);
        lv_obj_set_size(overlay, 128, 64);
        lv_obj_set_style_bg_color(overlay, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(overlay, LV_OPA_COVER, 0);

        lv_obj_t *title = lv_label_create(overlay);
        lv_label_set_text(title, "OTA Update");
        lv_obj_set_style_text_color(title, lv_color_white(), 0);
        lv_obj_set_style_text_font(title, &lv_font_unscii_8, 0);
        lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 2);

        lv_obj_t *info = lv_label_create(overlay);
        char infoBuf[32];
        snprintf(infoBuf, sizeof(infoBuf), "New: v%s", remoteVer.c_str());
        lv_label_set_text(info, infoBuf);
        lv_obj_set_style_text_color(info, lv_color_white(), 0);
        lv_obj_set_style_text_font(info, &lv_font_unscii_8, 0);
        lv_obj_align(info, LV_ALIGN_TOP_MID, 0, 16);

        lv_obj_t *prompt = lv_label_create(overlay);
        lv_label_set_text(prompt, "Update now?");
        lv_obj_set_style_text_color(prompt, lv_color_white(), 0);
        lv_obj_set_style_text_font(prompt, &lv_font_unscii_8, 0);
        lv_obj_align(prompt, LV_ALIGN_TOP_MID, 0, 28);

        // Selection labels: use > marker for selected item
        lv_obj_t *lblYes = lv_label_create(overlay);
        lv_obj_set_style_text_color(lblYes, lv_color_white(), 0);
        lv_obj_set_style_text_font(lblYes, &lv_font_unscii_8, 0);
        lv_obj_align(lblYes, LV_ALIGN_BOTTOM_LEFT, 20, -6);

        lv_obj_t *lblNo = lv_label_create(overlay);
        lv_obj_set_style_text_color(lblNo, lv_color_white(), 0);
        lv_obj_set_style_text_font(lblNo, &lv_font_unscii_8, 0);
        lv_obj_align(lblNo, LV_ALIGN_BOTTOM_RIGHT, -20, -6);

        // Default selection: No (sel=1)
        int sel = 1;
        lv_label_set_text(lblYes, " Yes");
        lv_label_set_text(lblNo, ">No");

        // Release GUI semaphore so LVGL can render
        extern SemaphoreHandle_t xGuiSemaphore;
        xSemaphoreGive(xGuiSemaphore);

        // Poll GPIO directly for Yes/No selection
        #define BTN_OK_PIN    32
        #define BTN_UP_PIN    33
        #define BTN_DOWN_PIN  34

        bool confirmed = false;
        while (!confirmed) {
            bool up = (digitalRead(BTN_UP_PIN) == LOW);
            bool down = (digitalRead(BTN_DOWN_PIN) == LOW);
            bool ok = (digitalRead(BTN_OK_PIN) == LOW);

            if ((up || down) && sel != (up ? 0 : 1)) {
                sel = up ? 0 : 1;
                lv_label_set_text(lblYes, sel == 0 ? ">Yes" : " Yes");
                lv_label_set_text(lblNo,  sel == 1 ? ">No"  : " No");
                delay(200);  // debounce
            }
            if (ok) {
                confirmed = true;
                delay(200);  // debounce
            }
            delay(20);
        }

        // Re-take semaphore
        xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);

        if (sel == 0) {  // Yes
            Serial.println("[OTA] User accepted, starting update...");

            // Show OTA progress screen
            lv_obj_clean(overlay);
            lv_obj_t *otaTitle = lv_label_create(overlay);
            lv_label_set_text(otaTitle, "Updating...");
            lv_obj_set_style_text_color(otaTitle, lv_color_white(), 0);
            lv_obj_set_style_text_font(otaTitle, &lv_font_unscii_8, 0);
            lv_obj_align(otaTitle, LV_ALIGN_TOP_MID, 0, 8);

            lv_obj_t *bar = lv_bar_create(overlay);
            lv_obj_set_size(bar, 100, 12);
            lv_obj_align(bar, LV_ALIGN_CENTER, 0, 4);
            lv_bar_set_range(bar, 0, 100);
            lv_bar_set_value(bar, 0, LV_ANIM_OFF);
            lv_obj_set_style_bg_color(bar, lv_color_black(), 0);
            lv_obj_set_style_border_color(bar, lv_color_white(), 0);
            lv_obj_set_style_border_width(bar, 1, 0);
            lv_obj_set_style_bg_color(bar, lv_color_white(), LV_PART_INDICATOR);
            lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, LV_PART_INDICATOR);

            lv_obj_t *pctLabel = lv_label_create(overlay);
            lv_label_set_text(pctLabel, "0%");
            lv_obj_set_style_text_color(pctLabel, lv_color_white(), 0);
            lv_obj_set_style_text_font(pctLabel, &lv_font_unscii_8, 0);
            lv_obj_align(pctLabel, LV_ALIGN_BOTTOM_MID, 0, -8);

            static lv_obj_t *s_bar = nullptr;
            static lv_obj_t *s_pctLabel = nullptr;
            s_bar = bar;
            s_pctLabel = pctLabel;

            otaUpdater.setProgressCallback([](int percent) {
                if (s_bar) lv_bar_set_value(s_bar, percent, LV_ANIM_OFF);
                if (s_pctLabel) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "%d%%", percent);
                    lv_label_set_text(s_pctLabel, buf);
                }
            });

            xSemaphoreGive(xGuiSemaphore);
            otaUpdater.performUpdate();
            // Only reaches here on failure
            xSemaphoreTake(xGuiSemaphore, portMAX_DELAY);
            lv_obj_del(overlay);
            Serial.println("[OTA] Update failed!");
        } else {
            Serial.println("[OTA] User declined update.");
            lv_obj_del(overlay);
        }
    }
}

void App_Uninit()
{
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
}
