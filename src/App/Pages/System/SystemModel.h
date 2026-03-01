#ifndef SYSTEM_MODEL_H
#define SYSTEM_MODEL_H

#include "App/Common/DataProc/DataProc.h"

namespace Page
{
    class SystemModel
    {
    public:
        void Init();
        void Deinit();
        void Update();
        
    private:
        Account* account;
    };
}

#endif
