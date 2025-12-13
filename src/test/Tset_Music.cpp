#include  "Test.h"

MusicPlayer music(25); // 建立 MusicPlayer 物件，25 為輸出腳位

void TEST::Music_Init(void) {
    Serial.println("Music_Init: Initializing Music Test...");
    // 在這裡加入音樂測試的初始化程式碼
    // 初始化 ESP-Music，指定輸出腳位 (GPIO25)
    // 播放簡單音階
    // 建立一段簡單旋律
    Note melody[] = {
        {NOTE_C4, N4}, {NOTE_D4, N4}, {NOTE_E4, N4}, {NOTE_F4, N4},
        {NOTE_G4, N4}, {NOTE_A4, N4}, {NOTE_B4, N4}, {NOTE_C5, N4}
    };
    music.play(melody, sizeof(melody)/sizeof(Note));
    Serial.println("Music_Init: Music Test initialized.");
}
void TEST::Music_Loop(void) {
    // 在這裡加入需要在主迴圈中執行的音樂測試程式碼
    // 例如，檢查音樂是否播放完畢，或是觸發其他音效
        Serial.println("Music finished playing.");
        // 可以選擇重新播放或執行其他動作
}