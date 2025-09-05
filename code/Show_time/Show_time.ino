#include <Arduino.h>
#include <ShiftRegister74HC595.h> // Librería para registros de desplazamiento
#include <Adafruit_GFX.h>         // Librería gráfica base
#include <Fonts/FreeSans11pt7b.h> // Fuente para la matriz
#include <WiFi.h>
#include "time.h"

// === CONFIGURACIÓN DE WI-FI ===
const char* ssid = "MovistarFibra-4378A0";        // Cambia por tu SSID
const char* password = "12345678"; // Cambia por tu contraseña

// === CONFIGURACIÓN NTP ===
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // GMT-3 Argentina
const int daylightOffset_sec = 0;

// === CONFIGURACIÓN DE PINES Y MATRIZ ===
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

// === CLASE PARA LA MATRIZ ===
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
    for (int i = 0; i < MATRIX_ROWS; i++)
      for (int j = 0; j < BYTES_PER_ROW; j++)
        framebuffer[i][j] = 0;
  }
};

MatrixDisplay display;

// === FUNCIONES DE FILAS ===
void prenderFila(uint8_t fila) { digitalWrite(filas[fila], HIGH); }
void apagarFila(uint8_t fila) { digitalWrite(filas[fila], LOW); }

// === REFRESCO DE MATRIZ ===
void refrescarMatrizCompleta() {
  for (uint8_t fila = 0; fila < MATRIX_ROWS; fila++) {
    apagarFila((fila + MATRIX_ROWS - 1) % MATRIX_ROWS);
    for (uint8_t bit = 0; bit < MATRIX_COLS; bit++) {
      uint8_t colByte = bit / 8;
      uint8_t bitMask = 0x80 >> (bit % 8);
      bool ledOn = (framebuffer[fila][colByte] & bitMask) != 0;
      sr.set(bit, ledOn ? HIGH : LOW);
    }
    prenderFila(fila);
    delayMicroseconds(200);
    apagarFila(fila);
  }
}

// === FUNCION PARA OBTENER HORA ===
String getHora() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return "??:??";
  }
  char buffer[6];
  sprintf(buffer, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(buffer);
}

// === VARIABLES DE SCROLL ===
int16_t x = MATRIX_COLS;

void setup() {
  Serial.begin(115200);

  // Configura pines de filas
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    apagarFila(i);
  }
  sr.setAllLow();

  // Configura pantalla
  display.setFont(&FreeSans11pt7b);
  display.setTextColor(1);
  display.setTextWrap(false);

  // Conectar WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado");

  // Configura NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  static unsigned long lastScroll = 0;

  if (millis() - lastScroll > 100) { // Scroll cada 100ms
    lastScroll = millis();

    display.clear();
    display.setCursor(x, 17); // Ajusta altura de texto
    String hora = getHora();
    display.print(hora);

    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(hora, 0, 0, &x1, &y1, &w, &h);

    x--;
    if (x < -w) x = MATRIX_COLS;
  }

  refrescarMatrizCompleta();
}
