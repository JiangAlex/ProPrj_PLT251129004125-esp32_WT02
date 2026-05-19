#ifndef SYSTEM_VIEW_H
#define SYSTEM_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "SystemModel.h"
#include <lvgl.h>

namespace Page
{
    #define SYS_ITEM_COUNT 9

    enum SysFunc { SYS_FUNC_OK = 0, SYS_FUNC_BACK, SYS_FUNC_COUNT };

    class SystemView
    {
    public:
        void Create(lv_obj_t *root);
        void Delete();
        void UpdateView(SystemModel *model);
        void SetSelected(int index);
        int GetSelected() { return selectedIndex; }
        int GetItemCount() { return SYS_ITEM_COUNT; }

        void EnterFuncArea();
        void ExitFuncArea();
        void SetFuncSelected(int index);
        int GetFuncSelected() { return funcSelectedIndex; }
        bool IsInFuncArea() { return inFuncArea; }

        void SetEditMode(bool en);
        bool IsEditMode() { return editMode; }

    private:
        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_list = nullptr;
        lv_obj_t *ui_func_bar = nullptr;
        lv_obj_t *lbl_func = nullptr;
        lv_obj_t *ui_items[SYS_ITEM_COUNT];
        int selectedIndex = 0;
        int funcSelectedIndex = 0;
        bool inFuncArea = false;
        bool editMode = false;
    };
}

#endif
