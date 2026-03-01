#ifndef __RADIO_VIEW_H
#define __RADIO_VIEW_H

#include "../Page.h"

namespace Page
{

class RadioView
{
public:
    enum {
        RADIO_ITEM_CH,
        RADIO_ITEM_CTCSS,
        RADIO_ITEM_POWER,
        RADIO_ITEM_RSSI,
        RADIO_ITEM_VOL,
        RADIO_ITEM_CQL,
        RADIO_ITEM_COUNT
    };

    enum {
        RADIO_FUNC_SCAN,
        RADIO_FUNC_BACK,
        RADIO_FUNC_COUNT
    };

    struct UInfo {
        lv_obj_t* cont;
        lv_obj_t* infoList;
        lv_obj_t* infoItems[RADIO_ITEM_COUNT];
        lv_obj_t* funcBar;
        lv_obj_t* funcLabel;
        lv_obj_t* labelLogo;
        lv_anim_timeline_t* anim_timeline;
    } ui;

    void Create(lv_obj_t* root);
    void Delete();
    
    void UpdateValue(int index, int value);
    void UpdateFrequency(float freq);
    int GetValue(int index);

    void SetSelected(int index);
    int GetSelected();
    int GetFuncSelected() { return funcSelectedIndex; }
    void SetFuncSelected(int index);
    int GetItemCount();

    void SetEditMode(bool enable);
    bool IsEditMode();
    void SetScanning(bool scanning);

    void EnterFuncArea();
    void ExitFuncArea();
    bool IsInFuncArea();

    // Static data members
    static const char* itemLabels[RADIO_ITEM_COUNT];
    static const float ctcssTable[];
    static const int ctcssCount;

private:
    void UpdateDisplay();

    int selectedIndex;
    int funcSelectedIndex;
    bool isEditMode;
    bool inFuncArea;
    bool isScanning;
    int values[RADIO_ITEM_COUNT];
    float current_frequency;
};

}

#endif