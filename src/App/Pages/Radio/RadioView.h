#ifndef __RADIO_VIEW_H
#define __RADIO_VIEW_H

#include "../Page.h"

namespace Page
{

// Radio 設定項目數量
#define RADIO_ITEM_COUNT 6

// Radio 設定項目索引
enum RadioItemIndex {
    RADIO_ITEM_CH = 0,
    RADIO_ITEM_CTCSS,
    RADIO_ITEM_POWER,
    RADIO_ITEM_RSSI,
    RADIO_ITEM_VOL,
    RADIO_ITEM_CQL
};

// 功能區項目
enum RadioFuncIndex {
    RADIO_FUNC_BACK = 0,
    RADIO_FUNC_COUNT
};

class RadioView
{
public:
    void Create(lv_obj_t* root);
    void Delete();
    
    // 導航控制
    void SetSelected(int index);
    int GetSelected() { return selectedIndex; }
    int GetItemCount() { return RADIO_ITEM_COUNT; }
    
    // 模式控制
    void SetEditMode(bool enable);
    bool IsEditMode() { return isEditMode; }
    
    // 功能區控制
    void SetFuncSelected(int index);
    bool IsInFuncArea() { return inFuncArea; }
    void EnterFuncArea();
    void ExitFuncArea();
    
    // 數值更新
    void UpdateValue(int index, int value);
    void UpdateValueStr(int index, const char* str);
    int GetValue(int index);

public:
    struct
    {
        lv_obj_t* cont = nullptr;              // 主內容區
        lv_obj_t* infoList = nullptr;          // 資訊列表
        lv_obj_t* infoItems[RADIO_ITEM_COUNT] = {nullptr};   // 資訊項目
        lv_obj_t* funcBar = nullptr;           // 功能提示區
        lv_obj_t* funcLabel = nullptr;         // 功能標籤
        lv_obj_t* labelLogo = nullptr;         // 保留兼容

        lv_anim_timeline_t* anim_timeline = nullptr;
    } ui;

private:
    void UpdateDisplay();
    
    int selectedIndex = 0;          // 當前選中的項目
    int funcSelectedIndex = 0;      // 功能區選中項目
    bool isEditMode = false;        // 是否在編輯模式
    bool inFuncArea = false;        // 是否在功能區
    
    // 數值儲存
    int values[RADIO_ITEM_COUNT] = {15, 0, 1, -85, 4, 4};  // CH, CTCSS(index), Power, RSSI, VOL, CQL
    
    // CTCSS 頻率表（常用值）
    static const float ctcssTable[];
    static const int ctcssCount;
    
    // 標籤名稱
    static const char* itemLabels[RADIO_ITEM_COUNT];
};

}

#endif // !__VIEW_H
