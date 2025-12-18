#include "RadioView.h"
#include "App/Configs/Version.h"

using namespace Page;

// 菜單項目文字（不含前綴）
const char* RadioView::menuTexts[4] = {
    "Trekking",
    "Radio", 
    "System",
    "Status"
};

void RadioView::Create(lv_obj_t *root)
{   
    // 強制更新佈局
    lv_obj_update_layout(root);
    Serial.printf("[RadioView] root size: %d x %d\n", lv_obj_get_width(root), lv_obj_get_height(root));
    
    // 初始化 UI 成員
    ui.anim_timeline = nullptr;
    ui.labelLogo = nullptr;
    selectedIndex = 0;

    // 設置 root 樣式：黑色背景
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);  // 總高度 48px（38px 菜單 + 10px 底部）
    lv_obj_set_pos(root, 0, 16);     // StatusBar 下方
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // ========== 主菜單區域 (38px) ==========
    lv_obj_t* menuList = lv_obj_create(root);
    lv_obj_remove_style_all(menuList);
    lv_obj_set_size(menuList, 128, 38);
    lv_obj_set_pos(menuList, 0, 0);
    lv_obj_set_style_bg_opa(menuList, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(menuList, 0, 0);
    lv_obj_set_flex_flow(menuList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(menuList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(menuList, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(menuList, LV_SCROLL_SNAP_CENTER);
    ui.menuList = menuList;
    ui.cont = menuList;

    // 創建 4 個菜單項目（每項 9px 高）
    for (int i = 0; i < 4; i++) {
        lv_obj_t* item = lv_label_create(menuList);
        lv_obj_set_style_text_font(item, &lv_font_unscii_8, 0);
        lv_label_set_text(item, menuTexts[i]);  // 先設置基本文字，SetSelected 會更新
        lv_obj_set_width(item, 128);
        lv_obj_set_style_pad_left(item, 0, 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        
        // 白色文字，透明背景
        lv_obj_set_style_text_color(item, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        
        ui.menuItems[i] = item;
    }

    // ========== 底部提示區 (10px) ==========
    lv_obj_t* bottomBar = lv_obj_create(root);
    lv_obj_remove_style_all(bottomBar);
    lv_obj_set_size(bottomBar, 128, 10);
    lv_obj_set_pos(bottomBar, 0, 38);
    lv_obj_set_style_bg_opa(bottomBar, LV_OPA_TRANSP, 0);
    ui.bottomBar = bottomBar;

    lv_obj_t* dateLabel = lv_label_create(bottomBar);
    lv_obj_set_style_text_font(dateLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(dateLabel, lv_color_white(), 0);
    lv_label_set_text(dateLabel, "SAT 12/17");
    lv_obj_align(dateLabel, LV_ALIGN_CENTER, 0, 0);
    ui.dateLabel = dateLabel;

    // 設置初始選中項目
    SetSelected(0);
    
    Serial.printf("[RadioView] Menu created with %d items\n", 4);
}

void RadioView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= 4) index = 3;
    
    // 更新所有菜單項目文字（使用 ▶ 標記選中項）
    char buf[32];
    for (int i = 0; i < 4; i++) {
        if (i == index) {
            snprintf(buf, sizeof(buf), "> %s", menuTexts[i]);  // 選中項加前綴
        } else {
            snprintf(buf, sizeof(buf), "  %s", menuTexts[i]);  // 未選中項加空格對齊
        }
        lv_label_set_text(ui.menuItems[i], buf);
        lv_obj_set_style_text_color(ui.menuItems[i], lv_color_white(), 0);
        lv_obj_set_style_bg_opa(ui.menuItems[i], LV_OPA_TRANSP, 0);
    }
    
    selectedIndex = index;
    
    // 滾動到選中項目
    lv_obj_scroll_to_view(ui.menuItems[index], LV_ANIM_ON);
}

void RadioView::Delete()
{
    if (ui.anim_timeline)
    {
        lv_anim_timeline_del(ui.anim_timeline);
        ui.anim_timeline = nullptr;
    }
}
