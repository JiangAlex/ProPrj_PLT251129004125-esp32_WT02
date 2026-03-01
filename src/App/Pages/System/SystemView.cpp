#include "SystemView.h"

using namespace Page;

static const char* menu_items[] = {
    "Upgrade",
    "Group Settings",
    "Register",
    "Version"
};

void SystemView::Create(lv_obj_t *root)
{
    ui_root = root;
    lv_obj_clean(root);
    Serial.printf("[SystemView] root size before setup: %d x %d\n", lv_obj_get_width(root), lv_obj_get_height(root));
    

    // Root Container
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);  // Total height 48px (34px info area + 14px func area)
    lv_obj_set_pos(root, 0, 16);     // StatusBar 下方
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // 1. Main Content (Top 34px)
    ui_list = lv_obj_create(root);
    lv_obj_remove_style_all(ui_list);
    lv_obj_set_size(ui_list, 128, 34);
    lv_obj_set_pos(ui_list, 0, 0);
    lv_obj_set_style_bg_opa(ui_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(ui_list, 0, 0); // 移除內邊距，確保內容從頂部開始
    lv_obj_set_flex_flow(ui_list, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(ui_list, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(ui_list, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(ui_list, LV_SCROLL_SNAP_START);
    lv_obj_set_scrollbar_mode(ui_list, LV_SCROLLBAR_MODE_OFF); 

    // 創建 4 個菜單項目
    for (int i = 0; i < 4; i++) {
        lv_obj_t* item = lv_label_create(ui_list);
        lv_obj_set_style_text_font(item, &lv_font_unscii_8, 0);
        lv_label_set_text(item, ""); // Will be set by SetSelected
        lv_obj_set_width(item, 128);
        lv_obj_set_style_pad_left(item, 0, 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        
        // White text, transparent background
        lv_obj_set_style_text_color(item, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        
        ui_items[i] = item;
    }
    
    // 2. FuncBar (Bottom 14px)
    ui_func_bar = lv_obj_create(root);
    lv_obj_remove_style_all(ui_func_bar);
    lv_obj_set_size(ui_func_bar, 128, 14);
    lv_obj_align(ui_func_bar, LV_ALIGN_BOTTOM_LEFT, 0, 0); // Align to bottom of parent
    lv_obj_set_style_bg_opa(ui_func_bar, LV_OPA_TRANSP, 0);

    lbl_func = lv_label_create(ui_func_bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "  [OK]     [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);
    
    selectedIndex = 0;
    SetSelected(0);

    lv_obj_update_layout(root); // Force layout update for root and all children
    Serial.printf("[SystemView] root size after setup: %d x %d at (%d, %d)\n", lv_obj_get_width(root), lv_obj_get_height(root), lv_obj_get_x(root), lv_obj_get_y(root));
    Serial.printf("[SystemView] ui_list size: %d x %d at (%d, %d)\n", lv_obj_get_width(ui_list), lv_obj_get_height(ui_list), lv_obj_get_x(ui_list), lv_obj_get_y(ui_list));
    Serial.printf("[SystemView] ui_func_bar size: %d x %d at (%d, %d)\n", lv_obj_get_width(ui_func_bar), lv_obj_get_height(ui_func_bar), lv_obj_get_x(ui_func_bar), lv_obj_get_y(ui_func_bar));
}

void SystemView::Delete()
{
    if (ui_root) {
        lv_obj_clean(ui_root);
        ui_root = nullptr;
    }
}

void SystemView::UpdateView(SystemModel *model)
{
}

void SystemView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= 4) index = 3;
    selectedIndex = index;

    for(int i=0; i<4; i++) {
        if (i == selectedIndex) {
            lv_label_set_text_fmt(ui_items[i], "> %s", menu_items[i]);
            lv_obj_scroll_to_view(ui_items[i], LV_ANIM_OFF);
        } else {
            lv_label_set_text_fmt(ui_items[i], "  %s", menu_items[i]);
        }
    }
}
