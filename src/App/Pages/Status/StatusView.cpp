#include "StatusView.h"

using namespace Page;

void StatusView::Create(lv_obj_t *root)
{
    ui_root = root;
    lastModel = nullptr;
    lv_obj_clean(root);

    // Root Container
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);
    lv_obj_set_pos(root, 0, 16);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // 1. Main Content (Top 34px)
    ui_list = lv_obj_create(root);
    lv_obj_remove_style_all(ui_list);
    lv_obj_set_size(ui_list, 128, 34);
    lv_obj_set_pos(ui_list, 0, 0);
    lv_obj_set_style_bg_opa(ui_list, LV_OPA_TRANSP, 0);
    lv_obj_set_flex_flow(ui_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_pad_all(ui_list, 0, 0); // 移除內邊距，確保內容從頂部開始
    lv_obj_set_style_pad_row(ui_list, 2, 0);
    lv_obj_set_scroll_dir(ui_list, LV_DIR_VER); // 設置垂直滾動方向
    lv_obj_set_scroll_snap_y(ui_list, LV_SCROLL_SNAP_START); // 滾動時對齊到頂部
    lv_obj_set_scrollbar_mode(ui_list, LV_SCROLLBAR_MODE_OFF);

    const char* item_prefixes[] = {"TEMP:", "ALT:", "PRES:", "STEP:", "COMP:"};
    
    for (int i = 0; i < 5; i++) {
        lv_obj_t* item = lv_label_create(ui_list);
        lv_obj_set_style_text_font(item, &lv_font_unscii_8, 0);
        lv_label_set_text_fmt(item, "%s --", item_prefixes[i]);
        lv_obj_set_width(item, 128);
        lv_obj_set_style_pad_left(item, 0, 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        lv_obj_set_style_text_color(item, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        ui_items[i] = item;
    }

    // 2. FuncBar (Bottom 14px)
    ui_func_bar = lv_obj_create(root);
    lv_obj_remove_style_all(ui_func_bar);
    lv_obj_set_size(ui_func_bar, 128, 14);
    lv_obj_set_pos(ui_func_bar, 0, 34);
    lv_obj_set_style_bg_opa(ui_func_bar, LV_OPA_TRANSP, 0);

    lbl_func = lv_label_create(ui_func_bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "           [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);

    selectedIndex = 0;
}

void StatusView::Delete()
{
    if (ui_root) {
        lv_obj_clean(ui_root);
        ui_root = nullptr;
    }
}

void StatusView::UpdateView(StatusModel *model)
{
    if (model) {
        lastModel = model;
    }
    if (!lastModel || !ui_root) return;

    lv_label_set_text_fmt(ui_items[0], "%sTEMP: %.1f C", selectedIndex == 0 ? "> " : "  ", lastModel->GetTemp());
    lv_label_set_text_fmt(ui_items[1], "%sALT: %.0f m",   selectedIndex == 1 ? "> " : "  ", lastModel->GetAlt());
    lv_label_set_text_fmt(ui_items[2], "%sPRES: %.0f hPa",selectedIndex == 2 ? "> " : "  ", lastModel->GetPress());
    lv_label_set_text_fmt(ui_items[3], "%sSTEP: %d",      selectedIndex == 3 ? "> " : "  ", lastModel->GetStep());
    lv_label_set_text_fmt(ui_items[4], "%sCOMP: %s",      selectedIndex == 4 ? "> " : "  ", lastModel->GetCompass());
}

void StatusView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= 5) index = 4;

    if (selectedIndex == index) return;

    selectedIndex = index;

    lv_obj_scroll_to_view(ui_items[index], LV_ANIM_OFF);
    UpdateView(nullptr);
}
