#include "App.h"
#include "App/Common/HAL/HAL.h"
#include "App/Common/DataProc/DataProc.h"
#include "App/Pages/AppFactory.h"
#include "App/Pages/StatusBar/StatusBar.h"
#include "App/Utils/PageManager/PageManager.h"

using namespace Page;

#define ACCOUNT_SEND_CMD(ACT, CMD)                                         \
    do                                                                     \
    {                                                                      \
        DataProc::ACT##_Info_t info;                                       \
        DATA_PROC_INIT_STRUCT(info);                                       \
        info.cmd = DataProc::CMD;                                          \
        DataProc::Center()->AccountMain.Notify(#ACT, &info, sizeof(info)); \
    } while (0)
//extern lv_indev_t *indev_touchpad;

void App_Init()
{
    static AppFactory factory;
    static PageManager manager(&factory);
    /* Make sure the default group exists */
    if (!lv_group_get_default())
    {
        lv_group_t *group = lv_group_create();
        lv_group_set_default(group);
        //lv_indev_set_group(indev_touchpad, group);
    }

    /* Initialize the data processing node */
    // Initialize DataProc System after LVGL (requires lv_mem_alloc)
    Serial.println("Initializing DataProc System...");
    Serial.print("Free heap before DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    DataProc_Init();
    Serial.print("Free heap after DataProc init: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println("DataProc System initialized");
    //ACCOUNT_SEND_CMD(Storage, STORAGE_CMD_LOAD);
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_LOAD);

    /* Set screen style */

    /* Set root default style */

    /* Initialize resource pool */

    /* Initialize status bar */
    Page::StatusBar_Create(lv_layer_top());

    /* Initialize pages */
    //manager.Install("MenuPresenter", "Pages/Menu");
    //manager.SetGlobalLoadAnimType(PageManager::LOAD_ANIM_OVER_TOP);
    //manager.Push("Pages/Menu");
}

void App_Uninit()
{
    ACCOUNT_SEND_CMD(SysConfig, SYSCONFIG_CMD_SAVE);
    //ACCOUNT_SEND_CMD(Recorder,  RECORDER_CMD_STOP);
}