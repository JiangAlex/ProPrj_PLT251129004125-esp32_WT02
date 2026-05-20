#!/usr/bin/env python3
"""
OTA + GPX Server for ESP32.
- Serves firmware.bin and version file for OTA
- POST /gpx: receives GPX file, simplifies to 500 points, forwards to ESP32

Usage:
    python3 server.py [--esp32 192.168.131.123]
"""

import http.server
import os
import sys
import xml.etree.ElementTree as ET
import urllib.request
import urllib.parse
from io import BytesIO

PORT = 8080
DIRECTORY = os.path.dirname(os.path.abspath(__file__))
ESP32_IP = "192.168.131.123"
MAX_POINTS = 500


def simplify_gpx(gpx_data: bytes, max_points: int = MAX_POINTS) -> str:
    """Parse GPX XML, downsample to max_points, return simplified GPX string."""
    root = ET.fromstring(gpx_data)
    ns = {'gpx': 'http://www.topografix.com/GPX/1/1'}

    # Try with namespace first, then without
    trkpts = root.findall('.//{http://www.topografix.com/GPX/1/1}trkpt')
    if not trkpts:
        trkpts = root.findall('.//trkpt')

    wpts = root.findall('.//{http://www.topografix.com/GPX/1/1}wpt')
    if not wpts:
        wpts = root.findall('.//wpt')

    total = len(trkpts)
    print(f"[GPX] Original: {total} track points, {len(wpts)} waypoints")

    # Downsample track points
    if total > max_points:
        step = total / max_points
        indices = [int(i * step) for i in range(max_points)]
        # Always include last point
        if indices[-1] != total - 1:
            indices[-1] = total - 1
        sampled = [trkpts[i] for i in indices]
    else:
        sampled = trkpts

    # Build simplified GPX
    lines = ['<?xml version="1.0" encoding="UTF-8"?>']
    lines.append('<gpx version="1.1">')
    
    # Waypoints
    for wpt in wpts[:50]:  # Max 50 waypoints
        lat = wpt.get('lat', '0')
        lon = wpt.get('lon', '0')
        name_el = wpt.find('{http://www.topografix.com/GPX/1/1}name')
        if name_el is None:
            name_el = wpt.find('name')
        name = name_el.text if name_el is not None else "WP"
        lines.append(f'<wpt lat="{lat}" lon="{lon}"><name>{name}</name></wpt>')

    # Track points
    lines.append('<trk><trkseg>')
    for pt in sampled:
        lat = pt.get('lat', '0')
        lon = pt.get('lon', '0')
        ele_el = pt.find('{http://www.topografix.com/GPX/1/1}ele')
        if ele_el is None:
            ele_el = pt.find('ele')
        ele = ele_el.text if ele_el is not None else "0"
        lines.append(f'<trkpt lat="{lat}" lon="{lon}"><ele>{ele}</ele></trkpt>')
    lines.append('</trkseg></trk>')
    lines.append('</gpx>')

    result = '\n'.join(lines)
    print(f"[GPX] Simplified: {len(sampled)} points, {len(result)} bytes")
    return result


def forward_to_esp32(gpx_str: str, filename: str) -> str:
    """Send simplified GPX to ESP32 WebGUI /upload endpoint."""
    url = f"http://{ESP32_IP}/upload?name={urllib.parse.quote(filename, safe='')}"
    data = gpx_str.encode('utf-8')
    req = urllib.request.Request(url, data=data, method='POST',
                                 headers={'Content-Type': 'application/octet-stream'})
    try:
        with urllib.request.urlopen(req, timeout=10) as resp:
            return resp.read().decode()
    except Exception as e:
        return f"Error forwarding to ESP32: {e}"


class Handler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def do_POST(self):
        if self.path.startswith('/gpx'):
            content_length = int(self.headers.get('Content-Length', 0))
            gpx_data = self.rfile.read(content_length)

            # Get filename from query or header
            filename = "upload.gpx"
            if '?' in self.path:
                params = urllib.parse.parse_qs(self.path.split('?')[1])
                if 'name' in params:
                    filename = params['name'][0]

            try:
                simplified = simplify_gpx(gpx_data)
                result = forward_to_esp32(simplified, filename)
                self.send_response(200)
                self.send_header('Content-Type', 'text/plain')
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
                self.wfile.write(result.encode())
            except Exception as e:
                self.send_response(500)
                self.send_header('Content-Type', 'text/plain')
                self.end_headers()
                self.wfile.write(f"Error: {e}".encode())
        else:
            self.send_response(404)
            self.end_headers()

    def do_OPTIONS(self):
        self.send_response(200)
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'POST, GET, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')
        self.end_headers()

    def log_message(self, format, *args):
        print(f"[Server] {self.address_string()} - {format % args}")


if __name__ == "__main__":
    if '--esp32' in sys.argv:
        idx = sys.argv.index('--esp32')
        ESP32_IP = sys.argv[idx + 1]

    with http.server.HTTPServer(("0.0.0.0", PORT), Handler) as httpd:
        print(f"[Server] Serving on port {PORT}")
        print(f"[Server] ESP32 target: {ESP32_IP}")
        print(f"[Server] OTA: http://0.0.0.0:{PORT}/firmware.bin")
        print(f"[Server] GPX upload: POST http://0.0.0.0:{PORT}/gpx?name=file.gpx")
        print(f"[Server] Press Ctrl+C to stop")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\n[Server] Stopped")
