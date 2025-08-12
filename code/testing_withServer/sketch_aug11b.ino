#include <Arduino.h>
#include <WiFi.h>
#include <ShiftRegister74HC595.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h>

// ======================= CONFIGURACIÓN HARDWARE =======================
#define DATA_PIN 23
#define CLOCK_PIN 18
#define LATCH_PIN 19

#define MATRIX_ROWS 18
#define MATRIX_COLS 24
#define BYTES_PER_ROW (MATRIX_COLS / 8)

ShiftRegister74HC595<3> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);

const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33
};

uint8_t framebuffer[MATRIX_ROWS][BYTES_PER_ROW];

// ======================= CLASE DISPLAY =======================
class MatrixDisplay : public Adafruit_GFX {
public:
  MatrixDisplay() : Adafruit_GFX(MATRIX_COLS, MATRIX_ROWS) {}

  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= MATRIX_COLS || y < 0 || y >= MATRIX_ROWS) return;
    uint8_t bitMask = 0x80 >> (x % 8);
    uint8_t &byte = framebuffer[y][x / 8];
    if (color) byte |= bitMask;
    else byte &= ~bitMask;
  }

  void clear() {
    memset(framebuffer, 0, sizeof(framebuffer));
  }
};

MatrixDisplay display;
uint8_t filaActual = 0;

// ======================= REFRESCO =======================
void prenderFila(uint8_t fila) { digitalWrite(filas[fila], HIGH); }
void apagarFila(uint8_t fila) { digitalWrite(filas[fila], LOW); }

void refrescarMatriz() {
  static uint8_t filaAnterior = MATRIX_ROWS - 1;
  apagarFila(filaAnterior);

  for (uint8_t bit = 0; bit < MATRIX_COLS; bit++) {
    uint8_t colByte = bit / 8;
    uint8_t bitMask = 0x80 >> (bit % 8);
    bool ledOn = (framebuffer[filaActual][colByte] & bitMask) != 0;
    sr.set(bit, ledOn ? HIGH : LOW);
  }

  prenderFila(filaActual);
  filaAnterior = filaActual;
  filaActual++;
  if (filaActual >= MATRIX_ROWS) filaActual = 0;
  delayMicroseconds(1);
}

// ======================= WIFI Y SERVIDOR =======================
const char* ssid     = "iPhone Fran";
const char* password = "holaquetal";

WiFiServer server(80);

#define BUF_SIZE 512
char curMessage[BUF_SIZE] = "";
char newMessage[BUF_SIZE] = "";
bool newMessageAvailable = false;
bool invertMode = false;
bool scrollRight = false;
uint16_t scrollSpeed = 75;

const char WebResponse[] =
"HTTP/1.1 200 OK\n"
"Content-Type: text/html\n\n";

const char WebPage[] =
"<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
"<title>Mensaje</title>"
"<script>"
"function SendData(){"
"  var msg = encodeURIComponent(document.frm.Message.value);"
"  var SD  = document.frm.ScrollType.value;"
"  var I   = document.frm.Invert.value;"
"  var SP  = document.frm.Speed.value;"
"  window.location = \"/?MSG=\"+msg+\"&SD=\"+SD+\"&I=\"+I+\"&SP=\"+SP;"
"}"
"</script>"
"</head><body>"
"<h1>Cartel Inteligente</h1>"
"<form name=\"frm\">"
" Mensaje:<br><input type=\"text\" name=\"Message\" maxlength=\"255\"><br><br>"
" <input type=\"radio\" name=\"Invert\" value=\"0\" checked> Normal"
" <input type=\"radio\" name=\"Invert\" value=\"1\"> Inversa<br><br>"
" <input type=\"radio\" name=\"ScrollType\" value=\"L\" checked> Izquierda"
" <input type=\"radio\" name=\"ScrollType\" value=\"R\"> Derecha<br><br>"
" Velocidad:<br>Rápido<input type=\"range\" name=\"Speed\" min=\"10\" max=\"200\">Lento<br><br>"
"</form>"
"<button onclick=\"SendData()\">Enviar</button>"
"</body></html>";

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
      if (*p == '%' && isxdigit(*(p+1)) && isxdigit(*(p+2))) {
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
  if (p) scrollRight = (*(p+3) == 'R');

  p = strstr(req, "I=");
  if (p) invertMode = (*(p+2) == '1');

  p = strstr(req, "SP=");
  if (p) scrollSpeed = atoi(p+3);
}

// ======================= VARIABLES TEXTO =======================
int16_t xPos = MATRIX_COLS;

// ======================= SETUP =======================
void setup() {
  Serial.begin(115200);
  Serial.println("Iniciando...");

  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    apagarFila(i);
  }
  sr.setAllLow();

  display.setFont(&FreeSans12pt7b);
  display.setTextWrap(false);

  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado. IP: " + WiFi.localIP().toString());
  server.begin();
}

// ======================= LOOP =======================
void loop() {
  WiFiClient client = server.available();
  if (client) {
    char req[BUF_SIZE];
    int idx = 0;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (idx < BUF_SIZE - 1) req[idx++] = c;
        if (c == '\n' && strstr(req, "\r\n\r\n")) break;
      }
    }
    req[idx] = '\0';
    getData(req);
    client.print(WebResponse);
    client.print(WebPage);
    client.stop();

    if (newMessageAvailable) {
      strcpy(curMessage, newMessage);
      newMessageAvailable = false;
      Serial.println("Nuevo mensaje: " + String(curMessage));
    }
  }

  display.clear();
  display.setCursor(xPos, 17);
  display.setTextColor(invertMode ? 0 : 1);
  display.print(curMessage);

  refrescarMatriz();

  static unsigned long lastScroll = 0;
  if (millis() - lastScroll > scrollSpeed) {
    lastScroll = millis();
    if (scrollRight) xPos++;
    else xPos--;

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(curMessage, 0, 0, &x1, &y1, &w, &h);
    if (!scrollRight && xPos < -w) xPos = MATRIX_COLS;
    if (scrollRight && xPos > MATRIX_COLS) xPos = -w;
  }
}
