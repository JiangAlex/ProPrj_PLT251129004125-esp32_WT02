#ifdef ENABLE_WEB_GUI

#include "HAL.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// Virtual button state (accessed by lv_port_indev.cpp)
volatile uint8_t g_virtual_key = 0;       // 0=none, 1=OK, 2=UP, 3=DOWN
volatile bool g_virtual_pressed = false;
volatile uint32_t g_virtual_timestamp = 0;

static AsyncWebServer server(80);
static AsyncWebSocket ws("/ws");
static volatile bool framebuffer_dirty = false;
static bool server_started = false;

// HTML page embedded in PROGMEM
static const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html><head><meta charset="utf-8"><title>SSD1306 Web GUI</title>
<style>
body{background:#1a1a1a;display:flex;flex-direction:column;align-items:center;justify-content:center;height:100vh;margin:0;font-family:monospace;color:#0f0}
canvas{border:2px solid #333;image-rendering:pixelated}
#status{margin:10px;font-size:14px}
#keys{margin:10px;font-size:12px;color:#888}
</style></head><body>
<div id="status">Disconnected</div>
<canvas id="oled" width="512" height="256"></canvas>
<div id="keys">Keys: Enter=OK | ArrowUp=UP/BACK | ArrowDown=DOWN/FN</div>
<script>
const canvas=document.getElementById('oled'),ctx=canvas.getContext('2d'),status=document.getElementById('status');
ctx.fillStyle='#000';ctx.fillRect(0,0,512,256);
let ws,connected=false;
function connect(){
  ws=new WebSocket('ws://'+location.host+'/ws');
  ws.binaryType='arraybuffer';
  ws.onopen=()=>{connected=true;status.textContent='Connected';status.style.color='#0f0'};
  ws.onclose=()=>{connected=false;status.textContent='Disconnected';status.style.color='#f00';setTimeout(connect,2000)};
  ws.onmessage=(e)=>{if(e.data instanceof ArrayBuffer&&e.data.byteLength===1024)render(new Uint8Array(e.data))};
}
function render(buf){
  const img=ctx.createImageData(128,64);
  for(let page=0;page<8;page++){
    for(let col=0;col<128;col++){
      const b=buf[page*128+col];
      for(let bit=0;bit<8;bit++){
        const y=page*8+bit,x=col,px=(y*128+x)*4;
        const on=(b>>bit)&1;
        img.data[px]=img.data[px+1]=img.data[px+2]=on?255:0;
        img.data[px+3]=255;
      }
    }
  }
  ctx.putImageData(img,0,0);
  ctx.drawImage(canvas,0,0,128,64,0,0,512,256);
}
// Workaround: draw at native res then scale
const offscreen=document.createElement('canvas');offscreen.width=128;offscreen.height=64;
const offCtx=offscreen.getContext('2d');
function render(buf){
  const img=offCtx.createImageData(128,64);
  for(let page=0;page<8;page++){
    for(let col=0;col<128;col++){
      const b=buf[page*128+col];
      for(let bit=0;bit<8;bit++){
        const y=page*8+bit,x=col,px=(y*128+x)*4;
        const on=(b>>bit)&1;
        img.data[px]=img.data[px+1]=img.data[px+2]=on?255:0;
        img.data[px+3]=255;
      }
    }
  }
  offCtx.putImageData(img,0,0);
  ctx.imageSmoothingEnabled=false;
  ctx.drawImage(offscreen,0,0,512,256);
}
const keyMap={'Enter':1,'ArrowUp':2,'ArrowDown':3};
document.addEventListener('keydown',(e)=>{const k=keyMap[e.key];if(k&&connected){ws.send(new Uint8Array([k,1]));status.textContent='Key: '+e.key+' DOWN';e.preventDefault()}});
document.addEventListener('keyup',(e)=>{const k=keyMap[e.key];if(k&&connected){ws.send(new Uint8Array([k,0]));status.textContent='Connected';e.preventDefault()}});
connect();
</script>
<div style="margin:10px;display:flex;gap:10px;flex-wrap:wrap">
<div><input type="file" id="gpx" accept=".gpx"><button onclick="uploadGPX()">Upload GPX</button></div>
<div><input type="file" id="fw" accept=".bin"><button onclick="uploadFW()">Upload Firmware</button></div>
</div>
<div id="upstat" style="margin:5px;font-size:12px;color:#ff0"></div>
<script>
function uploadGPX(){const f=document.getElementById('gpx').files[0];if(!f)return;
const fd=new FormData();fd.append('file',f);document.getElementById('upstat').textContent='Uploading GPX...';
fetch('/upload',{method:'POST',body:fd}).then(r=>r.text()).then(t=>{document.getElementById('upstat').textContent=t}).catch(e=>{document.getElementById('upstat').textContent='Error: '+e})}
function uploadFW(){const f=document.getElementById('fw').files[0];if(!f)return;
const fd=new FormData();fd.append('file',f);document.getElementById('upstat').textContent='Uploading firmware...';
fetch('/ota',{method:'POST',body:fd}).then(r=>r.text()).then(t=>{document.getElementById('upstat').textContent=t}).catch(e=>{document.getElementById('upstat').textContent='Error: '+e})}
</script></body></html>
)rawliteral";

static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.printf("[WebGUI] Client #%u connected\n", client->id());
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.printf("[WebGUI] Client #%u disconnected\n", client->id());
    } else if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_BINARY && len == 2) {
            if (data[1] == 1) {
                g_virtual_key = data[0];
                g_virtual_pressed = true;
                g_virtual_timestamp = millis();
            }
        }
    }
}

// GPX parser: extract trkpt and wpt from GPX XML, save as binary to SPIFFS
#include <SPIFFS.h>
#include <Update.h>

#define GPX_BIN_PATH "/gpx_data.bin"
#define MAX_TRACK_PTS 500
#define MAX_WPTS 50
#define WPT_NAME_LEN 12

struct TrkPt { float lat; float lon; };
struct WptData { float lat; float lon; char name[WPT_NAME_LEN]; };

static bool parseGPX(const String &xml) {
    TrkPt *tracks = (TrkPt*)malloc(MAX_TRACK_PTS * sizeof(TrkPt));
    WptData *wpts = (WptData*)malloc(MAX_WPTS * sizeof(WptData));
    if (!tracks || !wpts) { free(tracks); free(wpts); return false; }

    int tc = 0, wc = 0;
    int pos = 0;

    // Parse trkpt
    while (tc < MAX_TRACK_PTS) {
        int idx = xml.indexOf("<trkpt", pos);
        if (idx < 0) break;
        int latIdx = xml.indexOf("lat=\"", idx);
        int lonIdx = xml.indexOf("lon=\"", idx);
        if (latIdx < 0 || lonIdx < 0) break;
        tracks[tc].lat = xml.substring(latIdx + 5, xml.indexOf("\"", latIdx + 5)).toFloat();
        tracks[tc].lon = xml.substring(lonIdx + 5, xml.indexOf("\"", lonIdx + 5)).toFloat();
        tc++;
        pos = idx + 6;
    }

    // Parse wpt
    pos = 0;
    while (wc < MAX_WPTS) {
        int idx = xml.indexOf("<wpt", pos);
        if (idx < 0) break;
        int latIdx = xml.indexOf("lat=\"", idx);
        int lonIdx = xml.indexOf("lon=\"", idx);
        if (latIdx < 0 || lonIdx < 0) break;
        wpts[wc].lat = xml.substring(latIdx + 5, xml.indexOf("\"", latIdx + 5)).toFloat();
        wpts[wc].lon = xml.substring(lonIdx + 5, xml.indexOf("\"", lonIdx + 5)).toFloat();
        // Get name
        int nameIdx = xml.indexOf("<name>", idx);
        int nameEnd = xml.indexOf("</name>", idx);
        memset(wpts[wc].name, 0, WPT_NAME_LEN);
        if (nameIdx > 0 && nameEnd > nameIdx && nameIdx < xml.indexOf("</wpt", idx)) {
            String n = xml.substring(nameIdx + 6, nameEnd);
            n.toCharArray(wpts[wc].name, WPT_NAME_LEN);
        } else {
            snprintf(wpts[wc].name, WPT_NAME_LEN, "WP%d", wc + 1);
        }
        wc++;
        pos = idx + 4;
    }

    // Save binary
    File f = SPIFFS.open(GPX_BIN_PATH, "w");
    if (!f) { free(tracks); free(wpts); return false; }
    uint16_t tc16 = tc, wc16 = wc;
    f.write((uint8_t*)&tc16, 2);
    f.write((uint8_t*)&wc16, 2);
    f.write((uint8_t*)tracks, tc * sizeof(TrkPt));
    f.write((uint8_t*)wpts, wc * sizeof(WptData));
    f.close();

    Serial.printf("[GPX] Parsed %d track pts, %d waypoints\n", tc, wc);
    free(tracks);
    free(wpts);
    return true;
}

static String gpxUploadBuf;
static size_t otaContentLen = 0;

void HAL::WebServer_Init() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    // GPX upload endpoint
    server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (parseGPX(gpxUploadBuf)) {
            request->send(200, "text/plain", "GPX OK: " + String(gpxUploadBuf.length()) + " bytes parsed");
        } else {
            request->send(500, "text/plain", "GPX parse failed");
        }
        gpxUploadBuf = "";
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
        if (index == 0) gpxUploadBuf = "";
        gpxUploadBuf += String((char*)data, len);
    });

    // OTA firmware upload endpoint
    server.on("/ota", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (Update.hasError()) {
            request->send(500, "text/plain", "OTA FAILED");
        } else {
            request->send(200, "text/plain", "OTA OK, rebooting...");
            delay(1000);
            ESP.restart();
        }
    }, [](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (index == 0) {
            Serial.printf("[OTA] Start: %s\n", filename.c_str());
            otaContentLen = request->contentLength();
            if (!Update.begin(otaContentLen)) {
                Update.printError(Serial);
            }
        }
        if (Update.isRunning()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("[OTA] Success: %u bytes\n", index + len);
            } else {
                Update.printError(Serial);
            }
        }
    });

    server.begin();
    server_started = true;
    Serial.printf("[WebGUI] http://%s\n", WiFi.localIP().toString().c_str());
}

void HAL::WebServer_SetFrameDirty() {
    framebuffer_dirty = true;
}

void HAL::WebServer_Update() {
    if (!server_started) return;

    // Auto-release virtual key after 150ms timeout
    if (g_virtual_pressed && (millis() - g_virtual_timestamp > 150)) {
        g_virtual_pressed = false;
    }

    // Push framebuffer if dirty and clients connected
    if (framebuffer_dirty && ws.count() > 0) {
        framebuffer_dirty = false;
        ws.binaryAll(u8g2.getBufferPtr(), 1024);
    }
    ws.cleanupClients();
}

#endif // ENABLE_WEB_GUI
