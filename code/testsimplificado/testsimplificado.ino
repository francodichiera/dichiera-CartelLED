#include <Arduino.h>
#include <WiFi.h>

// === PINS PARA 74HC595 ===
#define DATA_PIN 23
#define CLOCK_PIN 18
#define LATCH_PIN 19

// === PINS PARA FILAS (TIP122) ===
#define MATRIX_ROWS 20
const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33, 34, 35
};

// === PARÁMETROS DE SCROLL ===
const int MATRIX_COLS = 96;
uint8_t ledMatrix[MATRIX_ROWS][MATRIX_COLS];
int scrollOffset = 0;
unsigned long lastScroll = 0;
const unsigned long scrollInterval = 1000;  // ms entre pasos

// Envía 24 bits a los 74HC595
void enviarDatos(uint32_t data) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 23; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (data & (1UL << i)) ? HIGH : LOW);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

// Muestra un “frame” desplazado `offset` sobre ledMatrix
void mostrarFrame(int offset) {
  for (int fila = 0; fila < MATRIX_ROWS; fila++) {
    // Construyo un uint32_t con 24 bits de la matriz
    uint32_t bits = 0;
    for (int c = 0; c < 24; c++) {
      int col = (offset + c) % MATRIX_COLS;
      bits = (bits << 1) | (ledMatrix[fila][col] & 0x1);
    }
    // Activo fila, envío bits y desactivo
    digitalWrite(filas[fila], LOW);
    enviarDatos(bits);
    delayMicroseconds(500);
    digitalWrite(filas[fila], HIGH);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inicializando matriz 20x96 (datos de prueba)...");

  // Configuro pines shift register
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  // Configuro pines de filas
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }

  // Lleno la matriz con datos aleatorios 0/1
  randomSeed(analogRead(A0));
  for (int y = 0; y < MATRIX_ROWS; y++) {
    for (int x = 0; x < MATRIX_COLS; x++) {
      ledMatrix[y][x] = random(2);
    }
  }
}

void loop() {
  // Muestro el frame actual
  mostrarFrame(scrollOffset);

  // Avanzo el scroll cada scrollInterval ms
  if (millis() - lastScroll >= scrollInterval) {
    lastScroll = millis();
    scrollOffset = (scrollOffset + 1) % MATRIX_COLS;
  }
}
