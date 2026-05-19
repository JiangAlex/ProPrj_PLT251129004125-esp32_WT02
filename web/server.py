#!/usr/bin/env python3
"""
OTA Server for ESP32 firmware updates.
Serves firmware.bin and version file on port 8080.

Usage:
    python3 server.py

Before running:
    1. Build firmware: pio run
    2. Copy firmware: cp .pio/build/esp32dev/firmware.bin web/
    3. Update web/version with new version string
    4. Run this server: python3 web/server.py
"""

import http.server
import os

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))

class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def log_message(self, format, *args):
        print(f"[OTA Server] {self.address_string()} - {format % args}")

if __name__ == "__main__":
    with http.server.HTTPServer(("0.0.0.0", PORT), Handler) as httpd:
        print(f"[OTA Server] Serving on port {PORT}")
        print(f"[OTA Server] Directory: {DIRECTORY}")
        print(f"[OTA Server] Version URL: http://192.168.131.62:{PORT}/version")
        print(f"[OTA Server] Firmware URL: http://192.168.131.62:{PORT}/firmware.bin")
        print(f"[OTA Server] Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[OTA Server] Stopped")
