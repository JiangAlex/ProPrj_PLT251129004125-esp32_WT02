#ifndef TREKKING_VIEW_H
#define TREKKING_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "TrekkingModel.h"
#include <lvgl.h>

namespace Page
{
    class TrekkingView
    {
    public:
        enum {
            TREKKING_ITEM_TEMP,
            TREKKING_ITEM_ALT,
            TREKKING_ITEM_PRESS,
            TREKKING_ITEM_ASCENT,
            TREKKING_ITEM_DIST,
            TREKKING_ITEM_TIME,
            TREKKING_ITEM_STATUS,
            TREKKING_ITEM_COUNT
        };

        enum {
            TREKKING_FUNC_START,
            TREKKING_FUNC_BACK,
            TREKKING_FUNC_COUNT
        };

        void Create(lv_obj_t *root);
        void Delete();
        void UpdateView(TrekkingModel *model);
        void SetEntryMode(bool en);
        void SetSelected(int index);
        int GetSelected() { return selectedIndex; }
        int GetItemCount() { return TREKKING_ITEM_COUNT; }
        void EnterFuncArea();
        void ExitFuncArea();
        void SetFuncSelected(int index);
        int GetFuncSelected() { return funcSelectedIndex; }
        bool IsInFuncArea() { return inFuncArea; }
    private:
        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_list = nullptr;
        lv_obj_t *ui_func_bar = nullptr;
        lv_obj_t *ui_entry_label = nullptr;
        lv_obj_t *lbl_func = nullptr;

        lv_obj_t* ui_items[TREKKING_ITEM_COUNT];
        int selectedIndex;
        bool inFuncArea;
        int funcSelectedIndex;
        TrekkingModel* lastModel;

        lv_obj_t* CreateLabelItem(lv_obj_t* parent, const char* icon, const char* text);
    };
}

#endif