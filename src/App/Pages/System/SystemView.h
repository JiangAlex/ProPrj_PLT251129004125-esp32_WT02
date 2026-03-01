#ifndef SYSTEM_VIEW_H
#define SYSTEM_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "SystemModel.h"
#include <lvgl.h>

namespace Page
{
    class SystemView
    {
    public:
        void Create(lv_obj_t *root);
        void Delete();
        void UpdateView(SystemModel *model);
        void SetSelected(int index);
        int GetSelected() { return selectedIndex; }
        int GetItemCount() { return 4; }

    private:
        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_list = nullptr;
        lv_obj_t *ui_func_bar = nullptr;
        lv_obj_t *lbl_func = nullptr;
        
        lv_obj_t *ui_items[4];
        int selectedIndex;
    };
}

#endif
