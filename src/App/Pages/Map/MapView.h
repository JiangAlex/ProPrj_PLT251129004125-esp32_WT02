#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include "App/Utils/PageManager/PageBase.h"
#include "MapModel.h"
#include <lvgl.h>

namespace Page
{
    enum MapMode { MAP_MODE_PAN = 0, MAP_MODE_ZOOM };

    class MapView
    {
    public:
        void Create(lv_obj_t *root);
        void Delete();
        void Draw(MapModel *model, float centerLat, float centerLon, float zoom, int trackIdx, MapMode mode);

    private:
        void DrawTrack(MapModel *model, float minLat, float maxLat, float minLon, float maxLon);
        void DrawPosition(MapModel *model, float minLat, float maxLat, float minLon, float maxLon);
        void DrawWaypoints(MapModel *model, float minLat, float maxLat, float minLon, float maxLon);

        lv_obj_t *ui_root = nullptr;
        lv_obj_t *ui_canvas = nullptr;
        lv_obj_t *ui_mode = nullptr;

        static const int CW = 128;
        static const int CH = 64;
        static lv_color_t canvas_buf[];
    };
}

#endif
