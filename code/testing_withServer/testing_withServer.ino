#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ShiftRegister74HC595.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Preferences.h>

// === CREDENCIALES WiFi ===
const char *ssid = "MovistarFibra-4378A0";
const char *password = "12345678";

// === PINES ===
#define DATA_PIN 23
#define CLOCK_PIN 18
#define LATCH_PIN 19
#define MATRIX_ROWS 18
#define MATRIX_COLS 24
#define BYTES_PER_ROW (MATRIX_COLS / 8)

// Registros 74HC595
ShiftRegister74HC595<3> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);
const uint8_t filas[MATRIX_ROWS] = { 3, 1, 2, 4, 5, 12, 13, 14, 15, 16, 17, 21, 22, 25, 26, 27, 32, 33 };
uint8_t framebuffer[MATRIX_ROWS][BYTES_PER_ROW];

// Display framebuffer
class MatrixDisplay : public Adafruit_GFX {
public:
  MatrixDisplay()
    : Adafruit_GFX(MATRIX_COLS, MATRIX_ROWS) {}
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= MATRIX_COLS || y < 0 || y >= MATRIX_ROWS) return;
    uint8_t bitMask = 0x80 >> (x % 8);
    uint8_t &byte = framebuffer[y][x / 8];
    if (color) byte |= bitMask;
    else byte &= ~bitMask;
  }
  void clear() {
    for (int i = 0; i < MATRIX_ROWS; i++)
      for (int j = 0; j < BYTES_PER_ROW; j++) framebuffer[i][j] = 0;
  }
};
MatrixDisplay display;

// Funciones de filas
void prenderFila(uint8_t f) {
  digitalWrite(filas[f], HIGH);
}
void apagarFila(uint8_t f) {
  digitalWrite(filas[f], LOW);
}

// Multiplexado
void refrescarMatrizCompleta() {
  for (uint8_t f = 0; f < MATRIX_ROWS; f++) {
    apagarFila((f + MATRIX_ROWS - 1) % MATRIX_ROWS);
    for (uint8_t b = 0; b < MATRIX_COLS; b++) {
      uint8_t byteCol = b / 8;
      uint8_t bitMask = 0x80 >> (b % 8);
      sr.set(b, (framebuffer[f][byteCol] & bitMask) ? HIGH : LOW);
    }
    prenderFila(f);
    delayMicroseconds(200);
    apagarFila(f);
  }
}

// === SERVIDOR HTTP ===
WiFiServer server(80);
#define BUF_SIZE 512
char curMessage[BUF_SIZE] = "";
char newMessage[BUF_SIZE] = "";
bool newMessageAvailable = false;
bool scrollRight = false;
uint16_t scrollSpeed = 100;

// Preferences para guardar mensaje, velocidad y dirección
Preferences prefs;

const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
const char WebPage[] =
  "<!DOCTYPE html><html><head><meta charset='UTF-8'>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<title>Cartel Inteligente</title>"
  "<style>"
  "body{font-family:sans-serif;background:#f3f0ff;color:#2a1a66;text-align:center;margin:0;padding:0;}"
  "h1{background:linear-gradient(90deg,#3399ff,#9933ff);color:white;padding:15px 0;margin:0;font-size:1.5em;letter-spacing:1px;}"
  "form{padding:20px;background:linear-gradient(145deg,#e6e0ff,#ffffff);margin:20px auto;width:90%;max-width:420px;border-radius:15px;box-shadow:0 8px 20px rgba(0,0,0,0.15);text-align:center;transition:0.3s;}"
  "form:hover{box-shadow:0 10px 25px rgba(0,0,0,0.25);}"
  "input[type=text]{width:95%;padding:10px;margin:10px 0;border:2px solid #3399ff;border-radius:8px;transition:0.3s;outline:none;}"
  "input[type=text]:focus{border-color:#9933ff;box-shadow:0 0 8px rgba(153,51,255,0.4);}"
  "input[type=range]{-webkit-appearance:none;width:95%;height:10px;border-radius:5px;background:#ccc;outline:none;}"
  "input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:22px;height:22px;border-radius:50%;background:#3399ff;cursor:pointer;box-shadow:0 2px 6px rgba(0,0,0,0.3);transition:0.3s;}"
  "input[type=range]::-webkit-slider-thumb:hover{background:#9933ff;}"
  "button{padding:12px 25px;margin-top:10px;background:linear-gradient(90deg,#3399ff,#9933ff);color:white;border:none;border-radius:8px;cursor:pointer;box-shadow:0 4px 12px rgba(0,0,0,0.2);transition:0.3s;font-size:1em;}"
  "button:hover{background:linear-gradient(90deg,#0055aa,#6600cc);box-shadow:0 6px 15px rgba(0,0,0,0.3);}"
  "#speedVal{font-weight:bold;color:#2a1a66; display:inline-block; width:50px; text-align:center;}"
  "footer{margin-top:20px;padding:12px;color:#2a1a66;font-size:0.9em;opacity:0.7;}"
  "@media(max-width:500px){input[type=text],input[type=range]{width:95%;}}"
  "</style>"
  "<script>"
  "function SendData(){"
  "var msg=encodeURIComponent(document.frm.Message.value);"
  "var SD=document.frm.ScrollType.value;"
  "var SP=document.frm.Speed.value;"
  "window.location='/?MSG='+msg+'&SD='+SD+'&SP='+SP;"
  "}"
  "function updateSpeed(val){document.getElementById('speedVal').innerText=val;}"
  "</script></head>"
  "<body>"
  "<h1>Cartel Inteligente</h1>"
  "<form name='frm'>"
  "Mensaje:<br><input type='text' name='Message' maxlength='255'><br><br>"
  "<input type='radio' name='ScrollType' value='L' checked>Izquierda"
  "<input type='radio' name='ScrollType' value='R'>Derecha<br><br>"
  "Velocidad: <span id='speedVal'>100</span><br>"
  "<input type='range' name='Speed' min='10' max='200' value='100' oninput='updateSpeed(this.value)'><br><br>"
  "</form>"
  "<button onclick='SendData()'>Enviar</button>"
  "<footer>Realizado por FRANCO DICHIERA 2025</footer>"
  "</body></html>";

// Utilidad para parsear %xx
uint8_t htoi(char c) {
  c = toupper(c);
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

void getData(char *req) {
  char *p;
  p = strstr(req, "MSG=");
  if (p) {
    p += 4;
    char *dst = newMessage;
    while (*p && *p != '&') {
      if (*p == '%' && isxdigit(*(p + 1)) && isxdigit(*(p + 2))) {
        p++;
        *dst++ = (htoi(*p++) << 4) | htoi(*p++);
      } else {
        *dst++ = *p++;
      }
    }
    *dst = '\0';
    newMessageAvailable = strlen(newMessage) > 0;
  }
  p = strstr(req, "SD=");
  if (p) scrollRight = (*(p + 3) == 'R');
  p = strstr(req, "SP=");
  if (p) scrollSpeed = atoi(p + 3);
}

int16_t xPos = MATRIX_COLS;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < MATRIX_ROWS; i++) pinMode(filas[i], OUTPUT), apagarFila(i);
  sr.setAllLow();
  display.setFont(&FreeSans12pt7b);
  display.setTextColor(1);
  display.setTextWrap(false);

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());

  // mDNS
  if (MDNS.begin("cartelhuergo")) Serial.println("mDNS iniciado: cartelhuergo.local");
  else Serial.println("Error iniciando mDNS");

  server.begin();

  // Preferences
  prefs.begin("cartel", false);
  String savedMsg = prefs.getString("mensaje", "BIENVENIDOS ELECTRONICA EXPO HUERGO 2025");
  savedMsg.toCharArray(curMessage, BUF_SIZE);
  scrollSpeed = prefs.getUInt("speed", 100);
  scrollRight = prefs.getBool("dir", false);
}

void loop() {
  // Servidor
  WiFiClient client = server.available();
  static char req[BUF_SIZE];
  static int idx = 0;
  if (client) {
    while (client.connected() && client.available()) {
      char c = client.read();
      if (idx < BUF_SIZE - 1) req[idx++] = c;
      if (c == '\n' && strstr(req, "\r\n\r\n")) {
        req[idx] = '\0';
        getData(req);
        client.print(WebResponse);
        client.print(WebPage);
        client.stop();
        if (newMessageAvailable) {
          strcpy(curMessage, newMessage);
          newMessageAvailable = false;
          xPos = MATRIX_COLS;

          // Guardar en Preferences
          prefs.putString("mensaje", curMessage);
          prefs.putUInt("speed", scrollSpeed);
          prefs.putBool("dir", scrollRight);
        }
        idx = 0;
        break;
      }
    }
  }

  // Scroll y refresco
  static unsigned long lastScroll = 0;
  if (millis() - lastScroll > scrollSpeed) {
    lastScroll = millis();
    xPos += scrollRight ? 1 : -1;
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(curMessage, 0, 0, &x1, &y1, &w, &h);
    if (!scrollRight && xPos < -w) xPos = MATRIX_COLS;
    if (scrollRight && xPos > MATRIX_COLS) xPos = -w;
  }
  display.clear();
  display.setCursor(xPos, 17);
  display.print(curMessage);
  refrescarMatrizCompleta();
}
