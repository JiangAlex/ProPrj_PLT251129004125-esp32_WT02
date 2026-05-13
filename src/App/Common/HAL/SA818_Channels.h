#ifndef SA818_CHANNELS_H
#define SA818_CHANNELS_H

#include "HAL_Def.h"
#include <math.h>

// SA818 VHF Custom Channel Implementation
// VHF Band: 134-174 MHz

// Channel count limits
#define SA818_MAX_CHANNELS 20
#define SA818_MIN_CHANNEL 1
#define SA818_MAX_CHANNEL 20

// VHF Frequency bounds (MHz)
#define VHF_FREQ_MIN 134.0f
#define VHF_FREQ_MAX 174.0f
#define VHF_SIMPLEX_MIN 144.0f
#define VHF_SIMPLEX_MAX 148.0f
#define VHF_REPEATER_MIN 144.0f
#define VHF_REPEATER_MAX 148.0f

// CTCSS Tones (Hz) - Standard tones for VHF
static const float CTCSS_TONES[] = {
    67.0, 69.3, 71.9, 74.4, 77.0, 79.7, 82.5, 85.4, 88.5, 91.5,
    94.8, 97.4, 100.0, 103.5, 107.2, 110.9, 114.8, 118.8, 123.0, 127.3,
    131.8, 136.5, 141.3, 146.2, 151.4, 156.5, 162.2, 167.9, 173.8, 179.9,
    186.2, 192.8, 203.5, 210.7, 218.1, 225.7, 233.6, 241.8, 250.3, 257.9
};
static const int CTCSS_TONE_COUNT = sizeof(CTCSS_TONES) / sizeof(CTCSS_TONES[0]);

// CTCSS tone index definitions (for use in struct)
#define CTCSS_OFF 0
#define CTCSS_67HZ  1
#define CTCSS_100HZ 13
#define CTCSS_123HZ 18
#define CTCSS_151HZ 24
#define CTCSS_186HZ 30
#define CTCSS_250HZ 38

// Channel type enum
typedef enum {
    CH_SIMPLEX_RECEIVE = 0,  // Receive-only simplex (144MHz band)
    CH_SIMPLEX_TRANSMIT = 1, // Transmit-only simplex (145MHz band)
    CH_REPEATER_INPUT = 2,   // Repeater input (144MHz band)
    CH_REPEATER_OUTPUT = 3   // Repeater output (145MHz band)
} ChannelType_t;

// SA818 Custom Channel Structure
typedef struct {
    int channel;                // Channel number (1-20)
    const char* name;           // Channel name
    float freq_rx;              // RX frequency (MHz)
    float freq_tx;              // TX frequency (MHz)
    ChannelType_t type;         // Channel type
    uint8_t ctcss_rx;          // RX CTCSS tone index (0=off, 1-39=tone)
    uint8_t ctcss_tx;          // TX CTCSS tone index (0=off, 1-39=tone)
    SA818_PowerMode powerMode;  // Power mode (L-CH/H-CH)
} SA818_CustomChannel_t;

// 20 Pre-defined VHF Channels
// Simplex: 144-146MHz, Repeater inputs: 144-148MHz, Repeater outputs: 145-149MHz
static const SA818_CustomChannel_t VHF_CHANNELS[SA818_MAX_CHANNELS] = {
    // Channel 1-5: FM Simplex
    {1,  "SMX-1",  145.0000, 145.0000, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {2,  "SMX-2",  145.0250, 145.0250, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {3,  "SMX-3",  145.0500, 145.0500, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {4,  "SMX-4",  145.0750, 145.0750, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {5,  "SMX-5",  145.1000, 145.1000, CH_SIMPLEX_RECEIVE,  CTCSS_100HZ,  CTCSS_100HZ, SA818_LOW_POWER},
    
    // Channel 6-10: Repeater inputs/outputs (144MHz -> 145MHz offset)
    {6,  "RPT-IN", 144.6000, 145.6000, CH_REPEATER_INPUT,   CTCSS_100HZ,  CTCSS_OFF,    SA818_HIGH_POWER},
    {7,  "RPT-OUT",145.6000, 144.6000, CH_REPEATER_OUTPUT, CTCSS_OFF,    CTCSS_100HZ,  SA818_HIGH_POWER},
    {8,  "RPT-2I", 144.6250, 145.6250, CH_REPEATER_INPUT,   CTCSS_123HZ,  CTCSS_OFF,    SA818_HIGH_POWER},
    {9,  "RPT-2O", 145.6250, 144.6250, CH_REPEATER_OUTPUT,  CTCSS_OFF,    CTCSS_123HZ,  SA818_HIGH_POWER},
    {10, "RPT-3I", 144.6500, 145.6500, CH_REPEATER_INPUT,   CTCSS_151HZ,  CTCSS_OFF,    SA818_HIGH_POWER},
    
    // Channel 11-15: More simplex and repeater
    {11, "RPT-3O", 145.6500, 144.6500, CH_REPEATER_OUTPUT, CTCSS_OFF,    CTCSS_151HZ,  SA818_HIGH_POWER},
    {12, "CALL",   145.5000, 145.5000, CH_SIMPLEX_RECEIVE, CTCSS_123HZ,  CTCSS_123HZ, SA818_LOW_POWER},
    {13, "SMX-6",  145.1250, 145.1250, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {14, "SMX-7",  145.1500, 145.1500, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    {15, "SMX-8",  145.1750, 145.1750, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER},
    
    // Channel 16-20: 147MHz band repeaters
    {16, "RPT-4I", 147.6000, 147.0000, CH_REPEATER_INPUT,   CTCSS_100HZ,  CTCSS_OFF,    SA818_HIGH_POWER},
    {17, "RPT-4O", 147.0000, 147.6000, CH_REPEATER_OUTPUT,  CTCSS_OFF,    CTCSS_100HZ,  SA818_HIGH_POWER},
    {18, "RPT-5I", 147.6250, 147.0250, CH_REPEATER_INPUT,   CTCSS_151HZ,  CTCSS_OFF,    SA818_HIGH_POWER},
    {19, "RPT-5O", 147.0250, 147.6250, CH_REPEATER_OUTPUT,  CTCSS_OFF,    CTCSS_151HZ,  SA818_HIGH_POWER},
    {20, "SMX-9",  147.5000, 147.5000, CH_SIMPLEX_RECEIVE,  CTCSS_OFF,    CTCSS_OFF,    SA818_LOW_POWER}
};

// Get channel structure by channel number (1-20)
inline const SA818_CustomChannel_t* getChannel(int channel) {
    if (channel < SA818_MIN_CHANNEL || channel > SA818_MAX_CHANNEL) {
        return NULL;
    }
    return &VHF_CHANNELS[channel - 1];
}

// Get RX frequency for channel
inline float getChannelFreqRx(int channel) {
    const SA818_CustomChannel_t* ch = getChannel(channel);
    return ch ? ch->freq_rx : 0.0f;
}

// Get TX frequency for channel
inline float getChannelFreqTx(int channel) {
    const SA818_CustomChannel_t* ch = getChannel(channel);
    return ch ? ch->freq_tx : 0.0f;
}

// Get channel name
inline const char* getChannelName(int channel) {
    const SA818_CustomChannel_t* ch = getChannel(channel);
    return ch ? ch->name : "";
}

// Check if frequency is valid VHF
inline bool isValidVHFrequency(float freq) {
    return (freq >= VHF_FREQ_MIN && freq <= VHF_FREQ_MAX);
}

// Normalize frequency to 4 decimal places (12.5kHz channel spacing)
inline float normalizeFrequency(float freq) {
    if (!isValidVHFrequency(freq)) {
        return 0.0f;
    }
    // Round to nearest 0.0125 MHz (12.5 kHz)
    return roundf(freq * 80.0f) / 80.0f;
}

// Get CTCSS tone frequency from index
inline float getCTCSSFrequency(uint8_t toneIndex) {
    if (toneIndex == 0 || toneIndex > CTCSS_TONE_COUNT) {
        return 0.0f;
    }
    return CTCSS_TONES[toneIndex - 1];
}

// ========== Backward-compatible wrappers for existing HAL_SA818.cpp ==========
// Old API: getSA818Frequency(powerMode, channel)
// - SA818_LOW_POWER  → return RX frequency (L-CH band)
// - SA818_HIGH_POWER → return TX frequency (H-CH band) — for simplex, TX=RX
inline float getSA818Frequency(SA818_PowerMode powerMode, int channel) {
    const SA818_CustomChannel_t* ch = getChannel(channel);
    if (!ch) return 0.0f;
    (void)powerMode;  // new struct embeds power mode per channel
    // For backward compat: prefer TX freq if repeater, else RX=TX for simplex
    return ch->freq_tx > 0 ? ch->freq_tx : ch->freq_rx;
}

inline const char* getPowerModeName(SA818_PowerMode powerMode) {
    return (powerMode == SA818_LOW_POWER) ? "L-CH" : "H-CH";
}

inline const char* getFrequencyRange(SA818_PowerMode powerMode) {
#if !defined(DRA818_CONFIG_UHF) || (DRA818_CONFIG_UHF == 0)
    return (powerMode == SA818_LOW_POWER) ?
           "144.0000-148.0000 MHz" :
           "144.0000-148.0000 MHz";
#else
    return (powerMode == SA818_LOW_POWER) ?
           "409.7500-409.9875 MHz" :
           "430.1375-439.4375 MHz";
#endif
}

#endif // SA818_CHANNELS_H
