#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "StatusModel.h"
#include <lvgl.h>

namespace Page
{
    class StatusView
    {
    public:
        void Create(lv_obj_t *root);
        void Delete();
        void UpdateView(StatusModel *model);
        void SetSelected(int index);
        int GetSelected() { return selectedIndex; }

    private:
        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_list = nullptr;
        lv_obj_t *ui_func_bar = nullptr;
        lv_obj_t *lbl_func = nullptr;

        lv_obj_t *ui_items[5];
        int selectedIndex;
        StatusModel* lastModel;
    };
}

#endif
