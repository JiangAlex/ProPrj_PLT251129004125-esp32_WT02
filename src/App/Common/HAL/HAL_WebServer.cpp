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
document.addEventListener('keydown',(e)=>{const k=keyMap[e.key];if(k&&connected){ws.send(new Uint8Array([k,1]));e.preventDefault()}});
document.addEventListener('keyup',(e)=>{const k=keyMap[e.key];if(k&&connected){ws.send(new Uint8Array([k,0]));e.preventDefault()}});
connect();
</script></body></html>
)rawliteral";

static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                       AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->opcode == WS_BINARY && len == 2) {
            // data[0] = key_id (1=OK, 2=UP, 3=DOWN), data[1] = state (1=press, 0=release)
            g_virtual_key = data[0];
            g_virtual_pressed = (data[1] == 1);
            g_virtual_timestamp = millis();
        }
    }
}

void HAL::WebServer_Init() {
    ws.onEvent(onWsEvent);
    server.addHandler(&ws);
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
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

    // Auto-release virtual key after 200ms timeout
    if (g_virtual_pressed && (millis() - g_virtual_timestamp > 200)) {
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
