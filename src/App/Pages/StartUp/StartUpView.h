#ifndef __STARTUP_VIEW_H
#define __STARTUP_VIEW_H

#include "../Page.h"

namespace Page
{

class StartupView
{
public:
    void Create(lv_obj_t* root);
    void Delete();
    void SetSelected(int index);
    int GetSelected() { return selectedIndex; }
    int GetMenuCount() { return 5; }

public:
    struct
    {
        lv_obj_t* cont;
        lv_obj_t* menuList;
        lv_obj_t* menuItems[5];
        lv_obj_t* bottomBar;
        lv_obj_t* dateLabel;
        lv_obj_t* labelLogo;
        lv_anim_timeline_t* anim_timeline;
    } ui;

private:
    int selectedIndex = 0;
    static const char* menuTexts[5];
};

}

#endif
