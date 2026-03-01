#include "StatusModel.h"

using namespace Page;

void StatusModel::Init()
{
    account = new Account("StatusModel", DataProc::Center(), 0, this);
}

void StatusModel::Deinit()
{
    if (account) {
        delete account;
        account = nullptr;
    }
}

void StatusModel::Update()
{
}
