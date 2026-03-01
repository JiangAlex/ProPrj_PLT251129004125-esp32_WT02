#include "SystemModel.h"

using namespace Page;

void SystemModel::Init()
{
    account = new Account("SystemModel", DataProc::Center(), 0, this);
}

void SystemModel::Deinit()
{
    if (account) {
        delete account;
        account = nullptr;
    }
}

void SystemModel::Update()
{
}
