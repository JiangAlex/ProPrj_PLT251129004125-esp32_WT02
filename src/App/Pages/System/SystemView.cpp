#include "SystemView.h"

using namespace Page;

enum SysItem {
    SYS_WIFI = 0, SYS_IP, SYS_HEAP, SYS_BRIGHT, SYS_TIMEZONE,
    SYS_CLOCK_FMT, SYS_GPS, SYS_VERSION, SYS_OTA, SYS_RESET_WIFI, SYS_REBOOT
};

void SystemView::Create(lv_obj_t *root)
{
    ui_root = root;
    lv_obj_clean(root);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);
    lv_obj_set_pos(root, 0, 16);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // Main list (34px)
    ui_list = lv_obj_create(root);
    lv_obj_remove_style_all(ui_list);
    lv_obj_set_size(ui_list, 128, 34);
    lv_obj_set_pos(ui_list, 0, 0);
    lv_obj_set_style_bg_opa(ui_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(ui_list, 0, 0);
    lv_obj_set_flex_flow(ui_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(ui_list, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(ui_list, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(ui_list, LV_SCROLLBAR_MODE_OFF);

    for (int i = 0; i < SYS_ITEM_COUNT; i++) {
        ui_items[i] = lv_label_create(ui_list);
        lv_obj_set_style_text_font(ui_items[i], &lv_font_unscii_8, 0);
        lv_obj_set_style_text_color(ui_items[i], lv_color_white(), 0);
        lv_obj_set_width(ui_items[i], 128);
        lv_obj_set_style_pad_ver(ui_items[i], 1, 0);
        lv_label_set_text(ui_items[i], "");
    }

    // FuncBar (14px)
    ui_func_bar = lv_obj_create(root);
    lv_obj_remove_style_all(ui_func_bar);
    lv_obj_set_size(ui_func_bar, 128, 14);
    lv_obj_set_pos(ui_func_bar, 0, 34);
    lv_obj_set_style_bg_opa(ui_func_bar, LV_OPA_TRANSP, 0);

    lbl_func = lv_label_create(ui_func_bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "  [OK]     [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);

    selectedIndex = 0;
    inFuncArea = false;
    funcSelectedIndex = 0;
    editMode = false;
}

void SystemView::Delete()
{
    if (ui_root) { lv_obj_clean(ui_root); ui_root = nullptr; }
}

void SystemView::UpdateView(SystemModel *model)
{
    if (!model || !ui_root) return;

    const char* prefix;
    char buf[32];

    for (int i = 0; i < SYS_ITEM_COUNT; i++) {
        if (!inFuncArea && i == selectedIndex) {
            prefix = editMode ? "* " : "> ";
        } else {
            prefix = "  ";
        }
        switch (i) {
            case SYS_WIFI:
                snprintf(buf, sizeof(buf), "%sWiFi: %s", prefix, model->IsWiFiConnected() ? "ON" : "OFF");
                break;
            case SYS_IP:
                snprintf(buf, sizeof(buf), "%sIP:%s", prefix, model->GetIP().c_str());
                break;
            case SYS_HEAP:
                snprintf(buf, sizeof(buf), "%sHeap: %uKB", prefix, model->GetFreeHeap() / 1024);
                break;
            case SYS_BRIGHT:
                snprintf(buf, sizeof(buf), "%sBright: %d", prefix, model->GetBrightness());
                break;
            case SYS_TIMEZONE:
                snprintf(buf, sizeof(buf), "%sTZ: UTC%+d", prefix, model->GetTimezone());
                break;
            case SYS_CLOCK_FMT:
                snprintf(buf, sizeof(buf), "%sClock: %s", prefix, model->Is24Hour() ? "24H" : "12H");
                break;
            case SYS_GPS:
                snprintf(buf, sizeof(buf), "%sGPS: %d sat", prefix, model->GetGPSSatellites());
                break;
            case SYS_VERSION:
                snprintf(buf, sizeof(buf), "%sVer: %s", prefix, model->GetVersion());
                break;
            case SYS_OTA:
                if (model->IsOTAAvailable())
                    snprintf(buf, sizeof(buf), "%sOTA: NEW *", prefix);
                else
                    snprintf(buf, sizeof(buf), "%sOTA Update", prefix);
                break;
            case SYS_RESET_WIFI:
                snprintf(buf, sizeof(buf), "%sReset WiFi", prefix);
                break;
            case SYS_REBOOT:
                snprintf(buf, sizeof(buf), "%sReboot", prefix);
                break;
        }
        lv_label_set_text(ui_items[i], buf);
    }

    // FuncBar
    if (inFuncArea) {
        if (funcSelectedIndex == SYS_FUNC_OK)
            lv_label_set_text(lbl_func, "> [OK]     [BACK]");
        else
            lv_label_set_text(lbl_func, "  [OK]   > [BACK]");
    } else {
        lv_label_set_text(lbl_func, "  [OK]     [BACK]");
    }
}

void SystemView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= SYS_ITEM_COUNT) index = SYS_ITEM_COUNT - 1;
    selectedIndex = index;
    inFuncArea = false;
    lv_obj_scroll_to_view(ui_items[index], LV_ANIM_OFF);
}

void SystemView::EnterFuncArea() { inFuncArea = true; funcSelectedIndex = 0; }
void SystemView::ExitFuncArea() { inFuncArea = false; }
void SystemView::SetFuncSelected(int index) {
    if (index < 0) index = 0;
    if (index >= SYS_FUNC_COUNT) index = SYS_FUNC_COUNT - 1;
    funcSelectedIndex = index;
}
void SystemView::SetEditMode(bool en) { editMode = en; }
