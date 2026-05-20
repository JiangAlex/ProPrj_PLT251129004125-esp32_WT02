#include "AppFactory.h"
#include "StartUp/StartUp.h"
#include "Radio/Radio.h"
#include "Trekking/Trekking.h"
#include "Trekking/Profile.h"
#include "System/System.h"
#include "Status/Status.h"
#include "Map/Map.h"


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
    Serial.printf("[AppFactory] CreatePage request: '%s'\n", name);

    APP_CLASS_MATCH(Startup);
    APP_CLASS_MATCH(Radio);
    APP_CLASS_MATCH(Trekking);
    APP_CLASS_MATCH(Profile);
    APP_CLASS_MATCH(System);
    APP_CLASS_MATCH(Status);
    APP_CLASS_MATCH(Map);
    //APP_CLASS_MATCH(MenuPresenter);
    
    Serial.printf("[AppFactory] Error: Page '%s' not found in factory!\n", name);
    return nullptr;
}