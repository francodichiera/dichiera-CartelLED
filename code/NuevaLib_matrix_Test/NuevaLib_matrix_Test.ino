#include <Arduino.h>
#include <ShiftRegister74HC595.h>  // https://blog.timodenk.com/shift-register-arduino-library/


/*Este código permite prender o apagar un solo LED específico usando coordenadas (x, y).*/


// === CONFIGURACIÓN DE PINES ===
#define DATA_PIN  23  // DS
#define CLOCK_PIN 18  // SHCP
#define LATCH_PIN 19  // STCP

#define MATRIX_ROWS 20
#define MATRIX_COLS 24

// Instancia del shift register (3 x 74HC595 = 24 salidas)
ShiftRegister74HC595<3> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);

// === Pines de filas conectadas a TIP122 ===
const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33, 34, 35
};

// === FUNCIONES ===
void prender(uint8_t x, uint8_t y) {
  if (x >= MATRIX_COLS || y >= MATRIX_ROWS) return;

  sr.setAllLow();  // Apaga todas las columnas
  sr.set(x, HIGH); // Activa columna X

  // Apaga todas las filas
  for (int i = 0; i < MATRIX_ROWS; i++) {
    digitalWrite(filas[i], HIGH);
  }

  // Activa fila Y (la pone en LOW para completar circuito)
  digitalWrite(filas[y], LOW);
}

void apagar(uint8_t x, uint8_t y) {
  if (x >= MATRIX_COLS || y >= MATRIX_ROWS) return;

  sr.set(x, LOW);                // Apaga columna X
  digitalWrite(filas[y], HIGH); // Apaga fila Y
}

void setup() {
  Serial.begin(115200);

  // Configurar pines de filas como salidas
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);  // Apagar todo al inicio
  }

  // Shift register ya se inicializa con la librería
  sr.setAllLow();  // Apaga todas las columnas
}

void loop() {
  prender(5, 10);  // Prender LED en columna 5, fila 10
  delay(500);

  apagar(5, 10);   // Apagar el mismo LED
  delay(500);
}
