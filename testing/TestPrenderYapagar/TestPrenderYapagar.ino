#include <Arduino.h>

// Parámetros de tu matriz 20×24
#define MATRIX_ROWS 20
#define MATRIX_COLS 24

// Pines ESP32 para los 74HC595
#define DATA_PIN 23
#define CLOCK_PIN 18
#define LATCH_PIN 19

// Pines ESP32 para las bases de fila (TIP122)
const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33, 34, 35
};

// Variables de control de parpadeo
bool encendida = true;
unsigned long previousMillis = 0;

// Control de brillo (0 = apagado, 255 = máximo)
uint8_t brillo = 50;  // Puedes ajustar este valor

void setup() {
  // Pines del shift register
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Pines de las filas (TIP122)
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);  // Desactivadas inicialmente
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Parpadeo cada 5 segundos
  if (currentMillis - previousMillis >= 500) {
    previousMillis = currentMillis;
    encendida = !encendida;
  }

  if (encendida) {
    for (int f = 0; f < MATRIX_ROWS; f++) {
      digitalWrite(filas[f], LOW);  // Activar fila

      sendAllOn();  // Encender columnas

      // PWM por software (control de brillo)
      delayMicroseconds(map(brillo, 0, 255, 0, 1000));  // Tiempo ON

      digitalWrite(filas[f], HIGH);  // Desactivar fila
      sendAllOff();                  // Apagar columnas

      delayMicroseconds(1000 - map(brillo, 0, 255, 0, 1000));  // Tiempo OFF
    }
  } else {
    sendAllOff();  // Mantener columnas apagadas
    for (int i = 0; i < MATRIX_ROWS; i++) {
      digitalWrite(filas[i], HIGH);  // Todas las filas desactivadas
    }
    delay(10);
  }
}

// Enciende todas las columnas (24 bits en HIGH)
void sendAllOn() {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < 3; i++) {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0xFF);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

// Apaga todas las columnas (24 bits en LOW)
void sendAllOff() {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 0; i < 3; i++) {
    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0x00);
  }
  digitalWrite(LATCH_PIN, HIGH);
}