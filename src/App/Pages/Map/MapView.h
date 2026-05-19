#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "MapModel.h"
#include <lvgl.h>

namespace Page
{
    enum MapFunc { MAP_FUNC_WPT = 0, MAP_FUNC_BACK, MAP_FUNC_COUNT };

    class MapView
    {
    public:
        void Create(lv_obj_t *root);
        void Delete();
        void UpdateView(MapModel *model);

        void EnterFuncArea();
        void ExitFuncArea();
        void SetFuncSelected(int index);
        int GetFuncSelected() { return funcSelectedIndex; }
        bool IsInFuncArea() { return inFuncArea; }

    private:
        void DrawMap(MapModel *model);
        void DrawTrack(MapModel *model);
        void DrawPosition(MapModel *model);
        void DrawWaypoints(MapModel *model);

        const char* BearingToCardinal(float bearing);

        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_canvas = nullptr;
        lv_obj_t *ui_info = nullptr;
        lv_obj_t *ui_func_bar = nullptr;
        lv_obj_t *lbl_func = nullptr;

        static const int CW = 128;
        static const int CH = 34;
        static uint8_t canvas_buf[];

        int funcSelectedIndex = 0;
        bool inFuncArea = false;
    };
}

#endif
