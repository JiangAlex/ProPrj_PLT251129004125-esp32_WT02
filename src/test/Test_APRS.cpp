
#include "Test.h"

AX25Msg msg;
void TEST::MicroAPRS_Init(void) {
    Serial.println("MicroAPRS_Init: Initializing MicroAPRS...");

    // 範例：填寫 AX25Msg 結構並發送一個簡單封包
    AX25Msg msg;
    memset(&msg, 0, sizeof(msg));
    strncpy(msg.src.call, "YOURCALL", sizeof(msg.src.call)); // 你的呼號
    msg.src.ssid = 9;
    strncpy(msg.dst.call, "APRS", sizeof(msg.dst.call));     // 目的呼號
    msg.dst.ssid = 0;
    msg.rpt_count = 0; // 不經中繼
    strcpy((char*)msg.info, "=2233.12N/12012.34E-ESP32 TEST"); // 位置/訊息
    msg.len = strlen((char*)msg.info);

    // 呼叫發送函式（請依你的 LibAPRS 實作調整）
    // 例如：LibAPRS.sendPacket(&msg);

    Serial.println("MicroAPRS_Init: Test packet sent.");
}

void TEST::MicroAPRS_Loop(void) {
    // 在這裡可以放置需要在主迴圈中執行的測試程式碼
        static unsigned long lastTx = 0;
    if (millis() - lastTx > 30000) {
        Serial.println("Sending APRS packet...");
        APRS_sendPkt(&msg, sizeof(msg));   // 使用 APRS_sendPkt 並補上長度參數
        lastTx = millis();
    }
}
