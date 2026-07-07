#include "SA818.h"
#include <cstdarg>

SA818::SA818() : _serial(nullptr), _ptt_pin(-1)
{
    memset(_response_buffer, 0, sizeof(_response_buffer));
}

bool SA818::begin(HardwareSerial& serial_port, int ptt_pin)
{
    _serial = &serial_port;
    _ptt_pin = ptt_pin;

    // PTT pin setup
    if (_ptt_pin >= 0) {
        pinMode(_ptt_pin, OUTPUT);
        digitalWrite(_ptt_pin, HIGH); // Default to receive mode (SA818 is Active Low for TX)
    }

    // Manually test connection for more robustness
    clearSerialBuffer();
    _serial->print("AT+DMOCONNECT\r\n");
    _serial->flush();
    delay(100);

    if (readResponse()) {
        // For connection test, be lenient and just check for the command name in response.
        if (strstr(_response_buffer, "+DMOCONNECT") != nullptr) {
            return true;
        }
        // If we got a response but it wasn't the expected one, print it for debugging
        Serial.printf("SA818::begin: Unexpected response: '%s'\n", _response_buffer);
    } else {
        Serial.println("SA818::begin: No response from module.");
    }
    return false;
}

bool SA818::setGroup(double tx_freq_mhz, double rx_freq_mhz, float ctcss_freq, uint8_t squelch)
{
    // Format CTCSS frequency string (e.g., 123.0 -> "1230")
    char ctcss_str[6];
    snprintf(ctcss_str, sizeof(ctcss_str), "%04d", static_cast<int>(ctcss_freq * 10));

    // Format frequencies manually to avoid locale issues (comma vs dot) and ensure 4 decimal places
    char tx_freq_str[16];
    char rx_freq_str[16];
    
    int tx_int = (int)tx_freq_mhz;
    int tx_dec = (int)((tx_freq_mhz - tx_int) * 10000 + 0.5);
    snprintf(tx_freq_str, sizeof(tx_freq_str), "%d.%04d", tx_int, tx_dec);

    int rx_int = (int)rx_freq_mhz;
    int rx_dec = (int)((rx_freq_mhz - rx_int) * 10000 + 0.5);
    snprintf(rx_freq_str, sizeof(rx_freq_str), "%d.%04d", rx_int, rx_dec);

    // AT+DMOSETGROUP=0,433.5000,433.5000,1230,1,0000
    // The last "0000" is for digital ID, not used in analog mode.
    return executeCommand(
        "AT+DMOSETGROUP=0,%s,%s,%s,%d,0000",
        tx_freq_str,
        rx_freq_str,
        ctcss_str,
        squelch
    );
}

bool SA818::setVolume(uint8_t level)
{
    if (level < 1 || level > 8) {
        return false; // Invalid level
    }
    return executeCommand("AT+DMOSETVOLUME=%d", level);
}

int SA818::scanRSSI()
{
    if (_serial == nullptr) return -999;

    clearSerialBuffer();
    _serial->print("AT+DMORSQ\r\n");
    _serial->flush();
    delay(100);

    if (!readResponse()) {
        return -999; // Timeout or no response
    }

    // The response is in _response_buffer, now we parse it.
    // Expected response: "+DMORSQ: 1, -100"
    const char* prefix = "+DMORSQ:";
    char* rssi_ptr = strstr(_response_buffer, prefix);

    if (rssi_ptr != nullptr) {
        // Move pointer past the prefix and the first number + comma
        rssi_ptr += strlen(prefix);
        char* comma_ptr = strchr(rssi_ptr, ',');
        if (comma_ptr != nullptr) {
            return atoi(comma_ptr + 1); // Convert the part after comma to integer
        }
    }

    return -999; // Parsing failed
}

void SA818::transmit(bool on)
{
    if (_ptt_pin >= 0) {
        // SA818 PTT is Active Low (Low = Transmit, High = Receive)
        digitalWrite(_ptt_pin, on ? LOW : HIGH);
    }
}

// --- Private Methods ---

void SA818::clearSerialBuffer()
{
    if (_serial == nullptr) return;
    while (_serial->available()) {
        _serial->read();
    }
}

bool SA818::executeCommand(const char* command_format, ...)
{
    if (_serial == nullptr) return false;

    char command[128];
    va_list args;
    va_start(args, command_format);
    vsnprintf(command, sizeof(command), command_format, args);
    va_end(args);

    Serial.printf("SA818 TX: %s\n", command); // Debug output to verify command format

    clearSerialBuffer();
    _serial->print(command);
    _serial->print("\r\n");
    _serial->flush(); // Wait for the command to be sent

    // Per SOP, wait for module to process before expecting a response
    delay(100); // Crucial delay for SA818

    // The expected response is the command name with a "+0" or similar success code.
    // For example, "AT+DMOSETGROUP=..." should respond with "+DMOSETGROUP:0".
    char expected_response[32];
    const char* cmd_name = strstr(command, "AT+") + 3;
    
    // Extract only the command name (before '=' if present)
    char cmd_only[32];
    const char* eq_pos = strchr(cmd_name, '=');
    if (eq_pos) {
        size_t len = eq_pos - cmd_name;
        if (len >= sizeof(cmd_only)) len = sizeof(cmd_only) - 1;
        strncpy(cmd_only, cmd_name, len);
        cmd_only[len] = '\0';
    } else {
        strncpy(cmd_only, cmd_name, sizeof(cmd_only) - 1);
        cmd_only[sizeof(cmd_only) - 1] = '\0';
    }
    snprintf(expected_response, sizeof(expected_response), "+%s:0", cmd_only);

    if (readResponse()) {
        if (strstr(_response_buffer, expected_response) != nullptr) {
            return true;
        }
    }
    return false;
}

bool SA818::readResponse(unsigned long timeout)
{
    if (_serial == nullptr) return false;

    unsigned long start_time = millis();
    size_t pos = 0;
    memset(_response_buffer, 0, sizeof(_response_buffer));
    while (millis() - start_time < timeout) {
        if (_serial->available()) {
            char c = _serial->read();
            if (c == '\r') continue; // Ignore CR
            if (c == '\n') {
                if (pos > 0) return true; // End of line, success if we have content
            } else if (pos < sizeof(_response_buffer) - 1) {
                _response_buffer[pos++] = c;
            }
        }
    }
    return (pos > 0); // Timeout, but success if we have some content
}