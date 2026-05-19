# OTA Server

Place firmware.bin and version file here, then serve via HTTP.

## Usage

```bash
cd web/
python3 -m http.server 8080
```

ESP32 will check `http://192.168.131.62:8080/version` on WiFi connect.
If newer version found, user can trigger update from System > OTA Update.

## Update Steps

1. Build: `pio run`
2. Copy: `cp .pio/build/esp32dev/firmware.bin web/`
3. Update `web/version` with new version string (e.g., `v1.0.2`)
4. Serve: `python3 -m http.server 8080`
5. ESP32 will detect and prompt user
