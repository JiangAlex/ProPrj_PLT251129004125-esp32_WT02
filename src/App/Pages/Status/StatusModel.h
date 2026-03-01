#ifndef STATUS_MODEL_H
#define STATUS_MODEL_H

#include "App/Common/DataProc/DataProc.h"

namespace Page
{
    class StatusModel
    {
    public:
        void Init();
        void Deinit();
        void Update();
        
        float GetTemp() { return 25.5f; }
        float GetAlt() { return 120.0f; }
        float GetPress() { return 1013.2f; }
        int GetStep() { return 5678; }
        const char* GetCompass() { return "NW"; }

    private:
        Account* account;
    };
}

#endif
