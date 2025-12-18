#include "AppFactory.h"
#include "StartUp/StartUp.h"
#include "Radio/Radio.h"

#define APP_CLASS_MATCH(className)         \
    do                                     \
    {                                      \
        if (strcmp(name, #className) == 0) \
        {                                  \
            return new Page::className;    \
        }                                  \
    } while (0)

PageBase *AppFactory::CreatePage(const char *name)
{
    APP_CLASS_MATCH(Startup);
    APP_CLASS_MATCH(Radio);
    //APP_CLASS_MATCH(MenuPresenter);
    return nullptr;
}