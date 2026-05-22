#include "MapView.h"
#include <math.h>

using namespace Page;

lv_color_t MapView::canvas_buf[128 * 64];

void MapView::Create(lv_obj_t *root)
{
    ui_root = root;
    lv_obj_clean(root);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 64);
    lv_obj_set_pos(root, 0, 0); // Fullscreen, no StatusBar
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // Canvas fullscreen
    ui_canvas = lv_canvas_create(root);
    lv_canvas_set_buffer(ui_canvas, canvas_buf, CW, CH, LV_IMG_CF_TRUE_COLOR);
    lv_obj_set_pos(ui_canvas, 0, 0);

    // Mode indicator (top-right corner, drawn on canvas)
    ui_mode = nullptr; // We'll draw mode directly on canvas
}

void MapView::Delete()
{
    if (ui_root) { lv_obj_clean(ui_root); ui_root = nullptr; }
}

void MapView::Draw(MapModel *model, float centerLat, float centerLon, float zoom, int trackIdx, MapMode mode)
{
    if (!ui_canvas) return;

    // Calculate view bounds from center + zoom
    // zoom = degrees of latitude visible in the view
    float halfLat = zoom / 2.0f;
    float halfLon = (zoom / 2.0f) * (128.0f / 64.0f); // Aspect ratio correction
    float minLat = centerLat - halfLat;
    float maxLat = centerLat + halfLat;
    float minLon = centerLon - halfLon;
    float maxLon = centerLon + halfLon;

    // Clear canvas
    lv_canvas_fill_bg(ui_canvas, lv_color_black(), LV_OPA_COVER);

    // Draw track
    if (model->GetTrackCount() > 1) {
        DrawTrack(model, minLat, maxLat, minLon, maxLon);
    }

    // Draw waypoints
    if (model->GetWaypointCount() > 0) {
        DrawWaypoints(model, minLat, maxLat, minLon, maxLon);
    }

    // Draw GPS position
    DrawPosition(model, minLat, maxLat, minLon, maxLon);

    // Draw current track position marker (small square on track)
    if (model->GetTrackCount() > 1 && trackIdx >= 0 && trackIdx < model->GetTrackCount()) {
        TrackPoint *pts = model->GetTrackPoints();
        int px = (int)((pts[trackIdx].lon - minLon) / (maxLon - minLon) * (CW - 1));
        int py = (CH - 1) - (int)((pts[trackIdx].lat - minLat) / (maxLat - minLat) * (CH - 1));
        // Draw small crosshair
        for (int d = -2; d <= 2; d++) {
            if (px+d >= 0 && px+d < CW) lv_canvas_set_px_color(ui_canvas, px+d, py, lv_color_white());
            if (py+d >= 0 && py+d < CH) lv_canvas_set_px_color(ui_canvas, px, py+d, lv_color_white());
        }
    }

    // Draw mode indicator top-right
    const char *modeChar = (mode == MAP_MODE_PAN) ? "P" : "Z";
    // Draw simple character at top-right (pixel font approximation)
    // P = Pan, Z = Zoom - use canvas set_px for a 3x5 pixel letter
    int ox = CW - 5, oy = 1;
    if (mode == MAP_MODE_PAN) {
        // P shape
        lv_canvas_set_px_color(ui_canvas, ox, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+1, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+2, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+3, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+4, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+1, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+2, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+2, oy+1, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+1, oy+2, lv_color_white());
    } else {
        // Z shape
        lv_canvas_set_px_color(ui_canvas, ox, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+1, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+2, oy, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+2, oy+1, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+1, oy+2, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+3, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox, oy+4, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+1, oy+4, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, ox+2, oy+4, lv_color_white());
    }
}

void MapView::DrawTrack(MapModel *model, float minLat, float maxLat, float minLon, float maxLon)
{
    int count = model->GetTrackCount();
    TrackPoint *pts = model->GetTrackPoints();
    float rangeX = maxLon - minLon;
    float rangeY = maxLat - minLat;

    for (int i = 1; i < count; i++) {
        int x0 = (int)((pts[i-1].lon - minLon) / rangeX * (CW - 1));
        int y0 = (CH-1) - (int)((pts[i-1].lat - minLat) / rangeY * (CH - 1));
        int x1 = (int)((pts[i].lon - minLon) / rangeX * (CW - 1));
        int y1 = (CH-1) - (int)((pts[i].lat - minLat) / rangeY * (CH - 1));

        // Clip: skip if both points far outside
        if ((x0 < -CW && x1 < -CW) || (x0 > 2*CW && x1 > 2*CW)) continue;
        if ((y0 < -CH && y1 < -CH) || (y0 > 2*CH && y1 > 2*CH)) continue;

        // Bresenham line
        int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy;
        int steps = 0;
        while (steps++ < 300) {
            if (x0 >= 0 && x0 < CW && y0 >= 0 && y0 < CH)
                lv_canvas_set_px_color(ui_canvas, x0, y0, lv_color_white());
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }
}

void MapView::DrawPosition(MapModel *model, float minLat, float maxLat, float minLon, float maxLon)
{
    GPS_Info_t gps;
    model->GetGPSInfo(&gps);
    if (!gps.isValid) return;

    float rangeX = maxLon - minLon;
    float rangeY = maxLat - minLat;

    int px = (int)((gps.longitude - minLon) / rangeX * (CW - 1));
    int py = (CH-1) - (int)((gps.latitude - minLat) / rangeY * (CH - 1));

    // Clamp to screen edges
    if (px < 3) px = 3;
    if (px > CW - 4) px = CW - 4;
    if (py < 3) py = 3;
    if (py > CH - 4) py = CH - 4;

    // Draw arrow pointing in GPS course direction
    float rad = gps.course * 3.14159f / 180.0f; // 0=North, 90=East
    float cosA = cos(rad);
    float sinA = sin(rad);

    // Arrow: tip (forward), two tail points
    // North = up = -Y in screen coords
    int tipX = px + (int)(sinA * 3);
    int tipY = py - (int)(cosA * 3);
    int t1X = px - (int)(sinA * 2) - (int)(cosA * 1);
    int t1Y = py + (int)(cosA * 2) - (int)(sinA * 1);
    int t2X = px - (int)(sinA * 2) + (int)(cosA * 1);
    int t2Y = py + (int)(cosA * 2) + (int)(sinA * 1);

    // Draw 3 lines: tip-t1, tip-t2, t1-t2
    auto drawLine = [&](int x0, int y0, int x1, int y1) {
        int dx = abs(x1-x0), sx = x0<x1?1:-1;
        int dy = -abs(y1-y0), sy = y0<y1?1:-1;
        int err = dx+dy;
        for (int s=0; s<20; s++) {
            if (x0>=0 && x0<CW && y0>=0 && y0<CH)
                lv_canvas_set_px_color(ui_canvas, x0, y0, lv_color_white());
            if (x0==x1 && y0==y1) break;
            int e2=2*err;
            if (e2>=dy){err+=dy;x0+=sx;}
            if (e2<=dx){err+=dx;y0+=sy;}
        }
    };
    drawLine(tipX, tipY, t1X, t1Y);
    drawLine(tipX, tipY, t2X, t2Y);
    drawLine(t1X, t1Y, t2X, t2Y);
}

void MapView::DrawWaypoints(MapModel *model, float minLat, float maxLat, float minLon, float maxLon)
{
    float rangeX = maxLon - minLon;
    float rangeY = maxLat - minLat;
    Waypoint *wps = model->GetWaypoints();

    for (int i = 0; i < model->GetWaypointCount(); i++) {
        int px = (int)((wps[i].lon - minLon) / rangeX * (CW - 1));
        int py = (CH-1) - (int)((wps[i].lat - minLat) / rangeY * (CH - 1));
        if (px < 0 || px >= CW || py < 0 || py >= CH) continue;

        // Draw + cross
        lv_canvas_set_px_color(ui_canvas, px, py, lv_color_white());
        if (px > 0) lv_canvas_set_px_color(ui_canvas, px-1, py, lv_color_white());
        if (px < CW-1) lv_canvas_set_px_color(ui_canvas, px+1, py, lv_color_white());
        if (py > 0) lv_canvas_set_px_color(ui_canvas, px, py-1, lv_color_white());
        if (py < CH-1) lv_canvas_set_px_color(ui_canvas, px, py+1, lv_color_white());
    }
}
