#include <Arduino.h>

#define NUM_FILAS    20
#define NUM_COLUMNAS 24

#define DATA_PIN   23
#define LATCH_PIN  19
#define CLOCK_PIN  18

int filas[NUM_FILAS] = {
  3,  1,  2,  4,  5, 12, 13, 14, 15, 16,
  17, 21, 22, 25, 26, 27, 32, 33, 34, 35
};

// Fuente 5x7 solo para letras A-Z
const byte font5x7[][5] = {
  {0x7E, 0x11, 0x11, 0x11, 0x7E},  // A
  // ... otras letras si querés seguir...
};

void setup() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  for (int i = 0; i < NUM_FILAS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }
}

void loop() {
  // Muestra solo la letra A
  for (int f = 0; f < 7; f++) {
    enviarDatos(font5x7[0][f]);     // Letra A, columna f
    digitalWrite(filas[f], LOW);
    delayMicroseconds(1000);
    digitalWrite(filas[f], HIGH);
  }
}

// Solo los primeros 8 bits (prueba básica)
void enviarDatos(byte dato) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = NUM_COLUMNAS - 1; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (dato & (1 << i)) ? HIGH : LOW);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}
