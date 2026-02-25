#ifndef SA818_H
#define SA818_H

#include <Arduino.h>
#include "HAL_Def.h"

#define SA818_RESPONSE_BUFFER_SIZE 64
#define SA818_CMD_TIMEOUT_MS 1000

/**
 * @brief Class to control the SA818 UHF/VHF radio module via AT commands.
 * This class encapsulates all low-level serial communication, timing, and response parsing.
 */
class SA818
{
public:
    SA818();

    /**
     * @brief Initializes the SA818 module.
     * @param serial_port Reference to the HardwareSerial instance.
     * @param ptt_pin The GPIO pin for Push-to-Talk control.
     * @return True if initialization and connection test are successful, false otherwise.
     */
    bool begin(HardwareSerial& serial_port, int ptt_pin);

    /**
     * @brief Sets the radio group parameters (frequency, CTCSS, squelch, etc.).
     * @param tx_freq_mhz Transmit frequency in MHz (e.g., 433.5000).
     * @param rx_freq_mhz Receive frequency in MHz.
     * @param ctcss_freq CTCSS tone frequency in Hz (e.g., 123.0). 0 for off.
     * @param squelch Squelch level (1-8).
     * @return True if the command was acknowledged successfully.
     */
    bool setGroup(double tx_freq_mhz, double rx_freq_mhz, float ctcss_freq, uint8_t squelch);

    /**
     * @brief Sets the speaker volume.
     * @param level Volume level (1-8).
     * @return True if the command was acknowledged successfully.
     */
    bool setVolume(uint8_t level);

    /**
     * @brief Scans the current channel's RSSI (Received Signal Strength Indicator).
     * @return The RSSI value (typically a negative dBm value), or -999 on error.
     */
    int scanRSSI();

    /**
     * @brief Controls the PTT (Push-to-Talk) pin.
     * @param on True to transmit (PTT Low), False to receive (PTT High).
     */
    void transmit(bool on);

private:
    HardwareSerial* _serial;
    int _ptt_pin;
    char _response_buffer[SA818_RESPONSE_BUFFER_SIZE];

    bool executeCommand(const char* command_format, ...);
    bool readResponse(unsigned long timeout = SA818_CMD_TIMEOUT_MS);
    void clearSerialBuffer();
};

#endif // SA818_H