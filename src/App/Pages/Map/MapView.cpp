#include "MapView.h"

using namespace Page;

// Canvas buffer for 128x34 monochrome (1 bit per pixel)
uint8_t MapView::canvas_buf[128 * 34 / 8 + 128]; // Extra for LVGL alignment

void MapView::Create(lv_obj_t *root)
{
    ui_root = root;
    lv_obj_clean(root);
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, 128, 48);
    lv_obj_set_pos(root, 0, 16);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(root, 0, 0);

    // Canvas (top 24px for map drawing)
    ui_canvas = lv_canvas_create(root);
    lv_canvas_set_buffer(ui_canvas, canvas_buf, CW, 24, LV_IMG_CF_ALPHA_1BIT);
    lv_obj_set_pos(ui_canvas, 0, 0);
    lv_canvas_fill_bg(ui_canvas, lv_color_black(), LV_OPA_COVER);

    // Info label (10px below canvas)
    ui_info = lv_label_create(root);
    lv_obj_set_style_text_font(ui_info, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(ui_info, lv_color_white(), 0);
    lv_obj_set_pos(ui_info, 0, 24);
    lv_obj_set_width(ui_info, 128);
    lv_label_set_text(ui_info, "  GPS: No Fix");

    // FuncBar (14px)
    ui_func_bar = lv_obj_create(root);
    lv_obj_remove_style_all(ui_func_bar);
    lv_obj_set_size(ui_func_bar, 128, 14);
    lv_obj_set_pos(ui_func_bar, 0, 34);
    lv_obj_set_style_bg_opa(ui_func_bar, LV_OPA_TRANSP, 0);

    lbl_func = lv_label_create(ui_func_bar);
    lv_obj_set_style_text_font(lbl_func, &lv_font_unscii_8, 0);
    lv_obj_set_style_text_color(lbl_func, lv_color_white(), 0);
    lv_label_set_text(lbl_func, "  [WPT]    [BACK]");
    lv_obj_align(lbl_func, LV_ALIGN_LEFT_MID, 0, 0);

    inFuncArea = false;
    funcSelectedIndex = 0;
}

void MapView::Delete()
{
    if (ui_root) { lv_obj_clean(ui_root); ui_root = nullptr; }
}

void MapView::UpdateView(MapModel *model)
{
    if (!model || !ui_root) return;

    DrawMap(model);

    // Update info line
    GPS_Info_t gps;
    model->GetGPSInfo(&gps);

    char buf[32];
    if (model->GetWaypointCount() > 0 && gps.isValid) {
        float dist = model->GetDistanceToTarget();
        float bearing = model->GetBearingToTarget();
        Waypoint *wp = &model->GetWaypoints()[model->GetTargetWpt()];
        if (dist > 1000)
            snprintf(buf, sizeof(buf), "  %s %.1fkm %s", wp->name, dist/1000, BearingToCardinal(bearing));
        else
            snprintf(buf, sizeof(buf), "  %s %dm %s", wp->name, (int)dist, BearingToCardinal(bearing));
    } else if (gps.isValid) {
        snprintf(buf, sizeof(buf), "  %.4f,%.4f %dsat", gps.latitude, gps.longitude, gps.satellites);
    } else {
        snprintf(buf, sizeof(buf), "  GPS: No Fix (%d sat)", gps.satellites);
    }
    lv_label_set_text(ui_info, buf);

    // FuncBar
    if (inFuncArea) {
        if (funcSelectedIndex == MAP_FUNC_WPT)
            lv_label_set_text(lbl_func, "> [WPT]    [BACK]");
        else
            lv_label_set_text(lbl_func, "  [WPT]  > [BACK]");
    } else {
        lv_label_set_text(lbl_func, "  [WPT]    [BACK]");
    }
}

void MapView::DrawMap(MapModel *model)
{
    // Clear canvas
    lv_canvas_fill_bg(ui_canvas, lv_color_black(), LV_OPA_COVER);

    if (model->GetTrackCount() > 0) {
        DrawTrack(model);
    }
    if (model->GetWaypointCount() > 0) {
        DrawWaypoints(model);
    }
    DrawPosition(model);
}

void MapView::DrawTrack(MapModel *model)
{
    int count = model->GetTrackCount();
    if (count < 2) return;

    TrackPoint *pts = model->GetTrackPoints();

    // Calculate bounding box
    float minLat = pts[0].lat, maxLat = pts[0].lat;
    float minLon = pts[0].lon, maxLon = pts[0].lon;
    for (int i = 1; i < count; i++) {
        if (pts[i].lat < minLat) minLat = pts[i].lat;
        if (pts[i].lat > maxLat) maxLat = pts[i].lat;
        if (pts[i].lon < minLon) minLon = pts[i].lon;
        if (pts[i].lon > maxLon) maxLon = pts[i].lon;
    }

    // Add 10% margin
    float dLat = (maxLat - minLat) * 0.1f;
    float dLon = (maxLon - minLon) * 0.1f;
    if (dLat < 0.0001f) dLat = 0.001f;
    if (dLon < 0.0001f) dLon = 0.001f;
    minLat -= dLat; maxLat += dLat;
    minLon -= dLon; maxLon += dLon;

    float scaleX = (CW - 1) / (maxLon - minLon);
    float scaleY = (24 - 1) / (maxLat - minLat);

    // Draw lines
    lv_draw_line_dsc_t line_dsc;
    lv_draw_line_dsc_init(&line_dsc);
    line_dsc.color = lv_color_white();
    line_dsc.width = 1;

    for (int i = 1; i < count; i++) {
        lv_point_t p[2];
        p[0].x = (lv_coord_t)((pts[i-1].lon - minLon) * scaleX);
        p[0].y = 23 - (lv_coord_t)((pts[i-1].lat - minLat) * scaleY);
        p[1].x = (lv_coord_t)((pts[i].lon - minLon) * scaleX);
        p[1].y = 23 - (lv_coord_t)((pts[i].lat - minLat) * scaleY);
        lv_canvas_draw_line(ui_canvas, p, 2, &line_dsc);
    }
}

void MapView::DrawPosition(MapModel *model)
{
    GPS_Info_t gps;
    model->GetGPSInfo(&gps);
    if (!gps.isValid) return;

    int count = model->GetTrackCount();
    if (count < 2) {
        // No track: draw position at center
        lv_canvas_set_px_color(ui_canvas, 64, 12, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, 63, 13, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, 64, 13, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, 65, 13, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, 64, 14, lv_color_white());
        return;
    }

    // Map GPS position to canvas using same bounding box as track
    TrackPoint *pts = model->GetTrackPoints();
    float minLat = pts[0].lat, maxLat = pts[0].lat;
    float minLon = pts[0].lon, maxLon = pts[0].lon;
    for (int i = 1; i < count; i++) {
        if (pts[i].lat < minLat) minLat = pts[i].lat;
        if (pts[i].lat > maxLat) maxLat = pts[i].lat;
        if (pts[i].lon < minLon) minLon = pts[i].lon;
        if (pts[i].lon > maxLon) maxLon = pts[i].lon;
    }
    float dLat = (maxLat - minLat) * 0.1f;
    float dLon = (maxLon - minLon) * 0.1f;
    if (dLat < 0.0001f) dLat = 0.001f;
    if (dLon < 0.0001f) dLon = 0.001f;
    minLat -= dLat; maxLat += dLat;
    minLon -= dLon; maxLon += dLon;

    int px = (int)((gps.longitude - minLon) / (maxLon - minLon) * (CW - 1));
    int py = 23 - (int)((gps.latitude - minLat) / (maxLat - minLat) * 23);

    // Draw ▲ (3x3 triangle)
    if (px >= 0 && px < CW && py >= 1 && py < 23) {
        lv_canvas_set_px_color(ui_canvas, px, py-1, lv_color_white());
        if (px > 0) lv_canvas_set_px_color(ui_canvas, px-1, py, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, px, py, lv_color_white());
        if (px < CW-1) lv_canvas_set_px_color(ui_canvas, px+1, py, lv_color_white());
        if (px > 0) lv_canvas_set_px_color(ui_canvas, px-1, py+1, lv_color_white());
        lv_canvas_set_px_color(ui_canvas, px, py+1, lv_color_white());
        if (px < CW-1) lv_canvas_set_px_color(ui_canvas, px+1, py+1, lv_color_white());
    }
}

void MapView::DrawWaypoints(MapModel *model)
{
    int count = model->GetTrackCount();
    if (count < 2) return;

    TrackPoint *pts = model->GetTrackPoints();
    float minLat = pts[0].lat, maxLat = pts[0].lat;
    float minLon = pts[0].lon, maxLon = pts[0].lon;
    for (int i = 1; i < count; i++) {
        if (pts[i].lat < minLat) minLat = pts[i].lat;
        if (pts[i].lat > maxLat) maxLat = pts[i].lat;
        if (pts[i].lon < minLon) minLon = pts[i].lon;
        if (pts[i].lon > maxLon) maxLon = pts[i].lon;
    }
    float dLat = (maxLat - minLat) * 0.1f;
    float dLon = (maxLon - minLon) * 0.1f;
    if (dLat < 0.0001f) dLat = 0.001f;
    if (dLon < 0.0001f) dLon = 0.001f;
    minLat -= dLat; maxLat += dLat;
    minLon -= dLon; maxLon += dLon;

    Waypoint *wps = model->GetWaypoints();
    int target = model->GetTargetWpt();

    for (int i = 0; i < model->GetWaypointCount(); i++) {
        int px = (int)((wps[i].lon - minLon) / (maxLon - minLon) * (CW - 1));
        int py = 23 - (int)((wps[i].lat - minLat) / (maxLat - minLat) * 23);
        if (px < 0 || px >= CW || py < 0 || py >= 24) continue;

        // Draw dot (target = larger)
        lv_canvas_set_px_color(ui_canvas, px, py, lv_color_white());
        if (i == target) {
            if (px > 0) lv_canvas_set_px_color(ui_canvas, px-1, py, lv_color_white());
            if (px < CW-1) lv_canvas_set_px_color(ui_canvas, px+1, py, lv_color_white());
            if (py > 0) lv_canvas_set_px_color(ui_canvas, px, py-1, lv_color_white());
            if (py < 23) lv_canvas_set_px_color(ui_canvas, px, py+1, lv_color_white());
        }
    }
}

const char* MapView::BearingToCardinal(float bearing)
{
    static const char* dirs[] = {"N","NE","E","SE","S","SW","W","NW"};
    int idx = (int)((bearing + 22.5f) / 45.0f) % 8;
    return dirs[idx];
}

void MapView::EnterFuncArea() { inFuncArea = true; funcSelectedIndex = 0; }
void MapView::ExitFuncArea() { inFuncArea = false; }
void MapView::SetFuncSelected(int index) {
    if (index < 0) index = 0;
    if (index >= MAP_FUNC_COUNT) index = MAP_FUNC_COUNT - 1;
    funcSelectedIndex = index;
}
