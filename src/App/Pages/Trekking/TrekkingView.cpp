#include "TrekkingView.h"

using namespace Page;

void TrekkingView::Create(lv_obj_t *root)
{
    ui_root = root;
    lastModel = nullptr;
    lv_obj_clean(root); // 清除 root 內的所有物件，確保頁面乾淨

    // 設置 root 樣式：黑色背景 (參照 RadioView)
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);  // Total height 48px (34px info area + 14px func area)
    lv_obj_set_pos(root, 0, 16);     // StatusBar 下方
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // 1. Create List Container (Top 34px - Main Content)
    ui_list = lv_obj_create(root);
    lv_obj_remove_style_all(ui_list); // 清除預設樣式 (避免白底)
    lv_obj_set_size(ui_list, 128, 34); // Main content area height
    lv_obj_set_pos(ui_list, 0, 0);
    lv_obj_set_style_bg_opa(ui_list, LV_OPA_TRANSP, 0); // 透明背景
    lv_obj_set_flex_flow(ui_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(ui_list, 0, 0);
    lv_obj_set_style_pad_row(ui_list, 2, 0);
    lv_obj_set_scroll_dir(ui_list, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(ui_list, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(ui_list, LV_SCROLLBAR_MODE_OFF); // 隱藏滾動條但保持滾動

    // Create List Items
    const char* item_labels[] = {"Temp:", "Alt:", "Pres:", "Asc:", "Dist:", "Time:", "Stat:", "GPX:", "Profile"};
    for (int i = 0; i < TREKKING_ITEM_COUNT; i++) {
        lv_obj_t* item = lv_label_create(ui_list);
        lv_obj_set_style_text_font(item, &lv_font_unscii_8, 0);
        lv_label_set_text_fmt(item, "  %s --", item_labels[i]);
        lv_obj_set_width(item, 128);
        lv_obj_set_style_pad_left(item, 0, 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        lv_obj_set_style_text_color(item, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        ui_items[i] = item;
    }
    selectedIndex = 0;
    inFuncArea = false;
    funcSelectedIndex = 0;

    // 2. Create FuncBar (Bottom 14px)
    // Style like RadioView
    ui_func_bar = lv_obj_create(root);
    lv_obj_remove_style_all(ui_func_bar); // 清除預設樣式
    lv_obj_set_size(ui_func_bar, 128, 14);
    lv_obj_align(ui_func_bar, LV_ALIGN_BOTTOM_MID, 0, 0); // More robustly align to bottom
    lv_obj_set_style_bg_opa(ui_func_bar, LV_OPA_TRANSP, 0);

    lbl_func = lv_label_create(ui_func_bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "  [START]   [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);

    // 3. Create Entry Label (Initially Hidden)
    ui_entry_label = lv_label_create(root);
    lv_obj_set_style_text_font(ui_entry_label, &lv_font_unscii_8, 0);
    lv_label_set_text(ui_entry_label, "TREKKING MODE\n\n[OK] to start");
    lv_obj_set_style_text_align(ui_entry_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(ui_entry_label, lv_color_white(), 0);
    // Align to center of Main Content (Height 34), relative to Root (Height 48)
    // Root center Y = 24. Main Content center Y = 17. Offset = 17 - 24 = -7.
    lv_obj_align(ui_entry_label, LV_ALIGN_CENTER, 0, -7);
}

void TrekkingView::Delete()
{
    if (ui_root)
    {
        lv_obj_clean(ui_root);
        ui_root = nullptr;
    }
}

void TrekkingView::UpdateView(TrekkingModel *model)
{
    if (model) {
        lastModel = model;
    }
    if (!lastModel || !ui_root) return;

    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_TEMP], "%sTemp: %.1f C", !inFuncArea && selectedIndex == TREKKING_ITEM_TEMP ? "> " : "  ", lastModel->GetTemperature());
    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_ALT], "%sAlt: %.0f m", !inFuncArea && selectedIndex == TREKKING_ITEM_ALT ? "> " : "  ", lastModel->GetAltitude());
    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_PRESS], "%sPres: %.0f hPa", !inFuncArea && selectedIndex == TREKKING_ITEM_PRESS ? "> " : "  ", lastModel->GetPressure());
    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_ASCENT], "%sAsc: %.0f m", !inFuncArea && selectedIndex == TREKKING_ITEM_ASCENT ? "> " : "  ", lastModel->GetAscent());
    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_DIST], "%sDist: %.2f km", !inFuncArea && selectedIndex == TREKKING_ITEM_DIST ? "> " : "  ", lastModel->GetDistanceKm());
    
    uint32_t t = lastModel->GetTimeMs() / 1000;
    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_TIME], "%sTime: %02d:%02d:%02d", !inFuncArea && selectedIndex == TREKKING_ITEM_TIME ? "> " : "  ", t / 3600, (t % 3600) / 60, t % 60);

    // Update status and func bar text
    const char* start_text;
    if (lastModel->IsRecording()) {
        lv_label_set_text_fmt(ui_items[TREKKING_ITEM_STATUS], "%sStat: RUNNING", !inFuncArea && selectedIndex == TREKKING_ITEM_STATUS ? "> " : "  ");
        start_text = "[PAUSE]";
    } else {
        if (lastModel->GetTimeMs() > 0) {
            lv_label_set_text_fmt(ui_items[TREKKING_ITEM_STATUS], "%sStat: PAUSED", !inFuncArea && selectedIndex == TREKKING_ITEM_STATUS ? "> " : "  ");
            start_text = "[RESUME]";
        } else {
            lv_label_set_text_fmt(ui_items[TREKKING_ITEM_STATUS], "%sStat: READY", !inFuncArea && selectedIndex == TREKKING_ITEM_STATUS ? "> " : "  ");
            start_text = "[START]";
        }
    }

    int gpxCount = lastModel->GetGPXFileCount();
    int gpxSel = lastModel->GetGPXSelected();
    const char* gpxPrefix = !inFuncArea && selectedIndex == TREKKING_ITEM_GPX ? "> " : "  ";
    if (gpxCount > 0)
        lv_label_set_text_fmt(ui_items[TREKKING_ITEM_GPX], "%sGPX: %d/%d", gpxPrefix, gpxSel + 1, gpxCount);
    else
        lv_label_set_text_fmt(ui_items[TREKKING_ITEM_GPX], "%sGPX: none", gpxPrefix);

    lv_label_set_text_fmt(ui_items[TREKKING_ITEM_PROFILE], "%sProfile >>", !inFuncArea && selectedIndex == TREKKING_ITEM_PROFILE ? "> " : "  ");

    char func_buf[32];
    if (inFuncArea) {
        if (funcSelectedIndex == TREKKING_FUNC_START) {
            snprintf(func_buf, sizeof(func_buf), "> %s   [BACK]", start_text);
        } else {
            snprintf(func_buf, sizeof(func_buf), "  %s > [BACK]", start_text);
        }
    } else {
        snprintf(func_buf, sizeof(func_buf), "  %s   [BACK]", start_text);
    }
    lv_label_set_text(lbl_func, func_buf);
}

void TrekkingView::SetEntryMode(bool en)
{
    if (en) {
        if (ui_entry_label) lv_obj_clear_flag(ui_entry_label, LV_OBJ_FLAG_HIDDEN);
        if (ui_list) lv_obj_add_flag(ui_list, LV_OBJ_FLAG_HIDDEN);
        if (ui_func_bar) lv_obj_add_flag(ui_func_bar, LV_OBJ_FLAG_HIDDEN);
    } else {
        if (ui_entry_label) lv_obj_add_flag(ui_entry_label, LV_OBJ_FLAG_HIDDEN);
        if (ui_list) lv_obj_clear_flag(ui_list, LV_OBJ_FLAG_HIDDEN);
        if (ui_func_bar) lv_obj_clear_flag(ui_func_bar, LV_OBJ_FLAG_HIDDEN);
    }
}

void TrekkingView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= TREKKING_ITEM_COUNT) index = TREKKING_ITEM_COUNT - 1;

    selectedIndex = index;
    inFuncArea = false;

    // 滾動到選中項目
    lv_obj_scroll_to_view(ui_items[index], LV_ANIM_OFF);
    UpdateView(nullptr);
}

void TrekkingView::EnterFuncArea()
{
    inFuncArea = true;
    funcSelectedIndex = 0;
    UpdateView(nullptr);
}

void TrekkingView::ExitFuncArea()
{
    inFuncArea = false;
    UpdateView(nullptr);
}

void TrekkingView::SetFuncSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= TREKKING_FUNC_COUNT) index = TREKKING_FUNC_COUNT - 1;
    
    funcSelectedIndex = index;
    UpdateView(nullptr);
}