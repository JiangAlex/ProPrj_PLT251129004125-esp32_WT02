#include "Test.h"

void TEST::Test_Init()
{
    TEST::ui_test_init (); // 初始化 UI 測試
    TEST::MicroAPRS_Init (); // 初始化 MicroAPRS 測試
    TEST::Music_Init (); // 初始化音樂測試
}