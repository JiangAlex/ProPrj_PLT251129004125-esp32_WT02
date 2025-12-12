#include "DataProc.h"
#include "../HAL/HAL.h"
#include "Arduino.h"
#include <new>

static DataCenter center("CENTER");

DataCenter* DataProc::Center()
{
    return &center;
}

// Static account storage to avoid dynamic allocation
static Account* accounts[13]; // 13 nodes including Sensors and StepCounter
static int accountIndex = 0;

void DataProc_Init()
{
    Serial.println("DataProc: Starting initialization...");
    Serial.print("Free heap before DataProc: ");
    Serial.println(ESP.getFreeHeap());
    
    accountIndex = 0;

    // Create accounts with careful memory management and smaller incremental steps
#define DP_DEF(NODE_NAME, BUFFER_SIZE)\
    Serial.print("DataProc: Creating " #NODE_NAME " ("); \
    Serial.print(BUFFER_SIZE); \
    Serial.print(" bytes)..."); \
    \
    /* Check available memory before allocation */ \
    if (ESP.getFreeHeap() < (BUFFER_SIZE + 1024)) { \
        Serial.println(" SKIP (insufficient memory)"); \
    } else { \
        Account* act##NODE_NAME = new(std::nothrow) Account(#NODE_NAME, &center, BUFFER_SIZE); \
        if (act##NODE_NAME != nullptr) { \
            accounts[accountIndex++] = act##NODE_NAME; \
            Serial.println(" OK"); \
        } else { \
            Serial.println(" FAILED (allocation failed)"); \
        } \
    } \
    delay(10); /* Small delay to allow system stability */
#  include "DP_LIST.inc"
#undef DP_DEF

    Serial.print("DataProc: Created ");
    Serial.print(accountIndex);
    Serial.println(" accounts successfully");
    
    Serial.print("Free heap after DataProc: ");
    Serial.println(ESP.getFreeHeap());
    Serial.println("DataProc: Initialization completed");
}

uint32_t DataProc::GetTick()
{
    return millis();
}

uint32_t DataProc::GetTickElaps(uint32_t prevTick)
{
    return millis() - prevTick;
}

const char* DataProc::MakeTimeString(uint64_t ms, char* buf, uint16_t len)
{
    if (!buf || len < 16) return nullptr;
    
    uint32_t seconds = ms / 1000;
    uint32_t minutes = seconds / 60;
    uint32_t hours = minutes / 60;
    
    seconds %= 60;
    minutes %= 60;
    hours %= 24;
    
    snprintf(buf, len, "%02lu:%02lu:%02lu", hours, minutes, seconds);
    return buf;
}
