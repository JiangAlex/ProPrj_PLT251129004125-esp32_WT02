#include "RadioView.h"
#include "App/Configs/Version.h"

using namespace Page;

// 項目標籤
const char* RadioView::itemLabels[RADIO_ITEM_COUNT] = {
    "CH",
    "CTCSS",
    "Power",
    "RSSI",
    "VOL",
    "CQL"
};

// CTCSS 頻率表（常用 CTCSS 音頻）
const float RadioView::ctcssTable[] = {
    67.0f, 71.9f, 74.4f, 77.0f, 79.7f, 82.5f, 85.4f, 88.5f,
    91.5f, 94.8f, 97.4f, 100.0f, 103.5f, 107.2f, 110.9f, 114.8f,
    118.8f, 123.0f, 127.3f, 131.8f, 136.5f, 141.3f, 146.2f, 151.4f,
    156.7f, 162.2f, 167.9f, 173.8f, 179.9f, 186.2f, 192.8f, 203.5f,
    210.7f, 218.1f, 225.7f, 233.6f, 241.8f, 250.3f
};
const int RadioView::ctcssCount = sizeof(ctcssTable) / sizeof(ctcssTable[0]);

void RadioView::Create(lv_obj_t *root)
{   
    // 強制更新佈局
    lv_obj_update_layout(root);
    Serial.printf("[RadioView] root size: %d x %d\n", lv_obj_get_width(root), lv_obj_get_height(root));
    
    // 初始化 UI 成員
    ui.anim_timeline = nullptr;
    ui.labelLogo = nullptr;
    selectedIndex = 0;
    isEditMode = false;
    inFuncArea = false;
    
    // 初始值
    values[RADIO_ITEM_CH] = 15;       // CH: 15
    values[RADIO_ITEM_CTCSS] = 7;     // CTCSS: index 7 = 88.5Hz
    values[RADIO_ITEM_POWER] = 1;     // Power: 1=HIGH, 0=LOW
    values[RADIO_ITEM_RSSI] = -85;    // RSSI: -85dBm
    values[RADIO_ITEM_VOL] = 4;       // VOL: 4
    values[RADIO_ITEM_CQL] = 4;       // CQL: 4

    // 設置 root 樣式：黑色背景
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);  // 總高度 48px（38px 資訊區 + 10px 功能區）
    lv_obj_set_pos(root, 0, 16);     // StatusBar 下方
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // ========== 主資訊區域 (38px) ==========
    lv_obj_t* infoList = lv_obj_create(root);
    lv_obj_remove_style_all(infoList);
    lv_obj_set_size(infoList, 128, 38);
    lv_obj_set_pos(infoList, 0, 0);
    lv_obj_set_style_bg_opa(infoList, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(infoList, 0, 0);
    lv_obj_set_flex_flow(infoList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(infoList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_scroll_dir(infoList, LV_DIR_VER);
    lv_obj_set_scroll_snap_y(infoList, LV_SCROLL_SNAP_CENTER);
    lv_obj_set_scrollbar_mode(infoList, LV_SCROLLBAR_MODE_OFF);  // 隱藏滾動條但保持滾動功能
    ui.infoList = infoList;
    ui.cont = infoList;

    // 創建 6 個資訊項目（每項 9px 高）
    for (int i = 0; i < RADIO_ITEM_COUNT; i++) {
        lv_obj_t* item = lv_label_create(infoList);
        lv_obj_set_style_text_font(item, &lv_font_unscii_8, 0);
        lv_label_set_text(item, "");
        lv_obj_set_width(item, 128);
        lv_obj_set_style_pad_left(item, 0, 0);
        lv_obj_set_style_pad_ver(item, 1, 0);
        
        // 白色文字，透明背景
        lv_obj_set_style_text_color(item, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(item, LV_OPA_TRANSP, 0);
        
        ui.infoItems[i] = item;
    }

    // ========== 功能提示區 (10px) ==========
    lv_obj_t* funcBar = lv_obj_create(root);
    lv_obj_remove_style_all(funcBar);
    lv_obj_set_size(funcBar, 128, 10);
    lv_obj_set_pos(funcBar, 0, 38);
    lv_obj_set_style_bg_opa(funcBar, LV_OPA_TRANSP, 0);
    ui.funcBar = funcBar;

    lv_obj_t* funcLabel = lv_label_create(funcBar);
    lv_obj_set_style_text_font(funcLabel, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(funcLabel, lv_color_white(), 0);
    lv_label_set_text(funcLabel, "  [BACK]");
    lv_obj_align(funcLabel, LV_ALIGN_LEFT_MID, 0, 0);
    ui.funcLabel = funcLabel;

    // 更新顯示
    UpdateDisplay();
    
    Serial.printf("[RadioView] Info items created: %d\n", RADIO_ITEM_COUNT);
}

void RadioView::UpdateDisplay()
{
    // 檢查 UI 是否已創建
    if (!ui.infoList || !ui.funcLabel) {
        return;
    }
    
    char buf[32];
    
    for (int i = 0; i < RADIO_ITEM_COUNT; i++) {
        if (!ui.infoItems[i]) {
            continue;  // 跳過未創建的項目
        }
        
        const char* prefix = "  ";
        const char* suffix = "";
        
        // 判斷是否選中（不在功能區時）
        if (!inFuncArea && i == selectedIndex) {
            if (isEditMode) {
                prefix = "* ";  // 編輯模式用 *
            } else {
                prefix = "> ";  // 導航模式用 >
            }
        }
        
        // 根據項目類型格式化值
        switch (i) {
            case RADIO_ITEM_CH:
                snprintf(buf, sizeof(buf), "%sCH: %d", prefix, values[i]);
                break;
            case RADIO_ITEM_CTCSS:
                // 索引 0 = OFF, 索引 1-38 對應 ctcssTable[0-37]
                if (values[i] == 0) {
                    snprintf(buf, sizeof(buf), "%sCTCSS: OFF", prefix);
                } else if (values[i] > 0 && values[i] <= ctcssCount) {
                    snprintf(buf, sizeof(buf), "%sCTCSS: %.1f", prefix, ctcssTable[values[i] - 1]);
                } else {
                    snprintf(buf, sizeof(buf), "%sCTCSS: ---", prefix);
                }
                break;
            case RADIO_ITEM_POWER:
                snprintf(buf, sizeof(buf), "%sPower: %s", prefix, values[i] ? "HIGH" : "LOW");
                break;
            case RADIO_ITEM_RSSI:
                snprintf(buf, sizeof(buf), "%sRSSI: %ddBm", prefix, values[i]);
                break;
            case RADIO_ITEM_VOL:
                snprintf(buf, sizeof(buf), "%sVOL: %d", prefix, values[i]);
                break;
            case RADIO_ITEM_CQL:
                snprintf(buf, sizeof(buf), "%sCQL: %d", prefix, values[i]);
                break;
        }
        
        lv_label_set_text(ui.infoItems[i], buf);
        lv_obj_set_style_text_color(ui.infoItems[i], lv_color_white(), 0);
    }
    
    // 更新功能區
    if (inFuncArea) {
        lv_label_set_text(ui.funcLabel, "> [BACK]");
    } else {
        lv_label_set_text(ui.funcLabel, "  [BACK]");
    }
}

void RadioView::SetSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= RADIO_ITEM_COUNT) index = RADIO_ITEM_COUNT - 1;
    
    selectedIndex = index;
    inFuncArea = false;
    
    UpdateDisplay();
    
    // 滾動到選中項目（使用 LV_ANIM_OFF 避免動畫導致的崩潰）
    if (ui.infoItems[index]) {
        lv_obj_scroll_to_view(ui.infoItems[index], LV_ANIM_OFF);
    }
}

void RadioView::SetEditMode(bool enable)
{
    isEditMode = enable;
    UpdateDisplay();
}

void RadioView::EnterFuncArea()
{
    inFuncArea = true;
    funcSelectedIndex = 0;
    UpdateDisplay();
}

void RadioView::ExitFuncArea()
{
    inFuncArea = false;
    UpdateDisplay();
}

void RadioView::SetFuncSelected(int index)
{
    if (index < 0) index = 0;
    if (index >= RADIO_FUNC_COUNT) index = RADIO_FUNC_COUNT - 1;
    
    funcSelectedIndex = index;
    UpdateDisplay();
}

void RadioView::UpdateValue(int index, int value)
{
    if (index < 0 || index >= RADIO_ITEM_COUNT) return;
    
    // 數值範圍限制
    switch (index) {
        case RADIO_ITEM_CH:
            if (value < 1) value = 1;
            if (value > 20) value = 20;
            break;
        case RADIO_ITEM_CTCSS:
            if (value < 0) value = ctcssCount - 1;  // 循環
            if (value >= ctcssCount) value = 0;
            break;
        case RADIO_ITEM_POWER:
            value = value ? 1 : 0;
            break;
        case RADIO_ITEM_VOL:
            if (value < 0) value = 0;
            if (value > 8) value = 8;
            break;
        case RADIO_ITEM_CQL:
            if (value < 0) value = 0;
            if (value > 8) value = 8;
            break;
        // RSSI 是只讀的，不修改
        case RADIO_ITEM_RSSI:
            return;
    }
    
    values[index] = value;
    UpdateDisplay();
}

int RadioView::GetValue(int index)
{
    if (index < 0 || index >= RADIO_ITEM_COUNT) return 0;
    return values[index];
}

void RadioView::Delete()
{
    // 停止所有動畫
    if (ui.anim_timeline)
    {
        lv_anim_timeline_del(ui.anim_timeline);
        ui.anim_timeline = nullptr;
    }
    
    // 清理 infoList 的滾動動畫
    if (ui.infoList) {
        lv_obj_scroll_to(ui.infoList, 0, 0, LV_ANIM_OFF);
    }
    
    // 清理所有資訊項目的動畫
    for (int i = 0; i < RADIO_ITEM_COUNT; i++) {
        if (ui.infoItems[i]) {
            lv_anim_del(ui.infoItems[i], NULL);
        }
    }
}
