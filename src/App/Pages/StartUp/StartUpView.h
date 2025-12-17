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
    int GetMenuCount() { return 4; }

public:
    struct
    {
        lv_obj_t* cont;           // 主內容區
        lv_obj_t* menuList;       // 菜單列表
        lv_obj_t* menuItems[4];   // 菜單項目
        lv_obj_t* bottomBar;      // 底部提示區
        lv_obj_t* dateLabel;      // 日期標籤
        lv_obj_t* labelLogo;      // 保留兼容

        lv_anim_timeline_t* anim_timeline;
    } ui;

private:
    int selectedIndex = 0;
    static const char* menuTexts[4];
};

}

#endif // !__VIEW_H
