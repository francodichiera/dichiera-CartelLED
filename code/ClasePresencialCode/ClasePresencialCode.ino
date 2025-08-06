#include <Arduino.h>
#include <ShiftRegister74HC595.h>  //https://blog.timodenk.com/shift-register-arduino-library/

  int numberOfShiftRegisters = 3;  // number of shift registers attached in series
  int serialDataPin = 23;          // DS
  int clockPin = 18;               // SHCP
  int latchPin = 19;               // STCP
  ShiftRegister74HC595<3> sr(serialDataPin, clockPin, latchPin);




// === PINS PARA 74HC595 ===
#define DATA_PIN 23
#define CLOCK_PIN 18
#define LATCH_PIN 19

// === MATRIZ ===
#define MATRIX_ROWS 20
#define MATRIX_COLS 24

byte led = 0;

const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33, 34, 35
};

// === MATRIZ DE ESTADO ===
uint8_t ledmatrix[MATRIX_ROWS][MATRIX_COLS] = { 0 };

// === FUNCIONES PARA UN SOLO PUNTO ===
void prender(uint8_t x, uint8_t y) {
  if (x < MATRIX_COLS && y < MATRIX_ROWS) {
    digitalWrite(x, LOW);
    digitalWrite(y, LOW);
  }
}

void apagar(uint8_t x, uint8_t y) {
  if (x < MATRIX_COLS && y < MATRIX_ROWS) {
    digitalWrite(x, HIGH);
    digitalWrite(y, HIGH);
  }
}

// === FUNCIONES DE CONTROL DE MATRIZ ===
void enviarDatos(uint32_t bits24) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 23; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (bits24 >> i) & 0x01);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void mostrar(int offset) {
  for (int y = 0; y < MATRIX_ROWS; y++) {
    uint32_t bits = 0;
    for (int x = 0; x < 24; x++) {
      int col = (offset + x) % MATRIX_COLS;
      bits = (bits << 1) | (ledmatrix[y][col] & 0x01);
    }
    digitalWrite(filas[y], LOW);
    enviarDatos(bits);
    delayMicroseconds(500);
    digitalWrite(filas[y], HIGH);
  }
}

void limpiar() {
  for (int y = 0; y < MATRIX_ROWS; y++) {
    for (int x = 0; x < MATRIX_COLS; x++) {
      ledmatrix[y][x] = 0;
    }
  }
}

// === SETUP Y LOOP ===

void setup() {
  Serial.begin(115200);





  // Pines de los registros
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  // Pines de filas
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], LOW);
  }
}

void loop() {

  // digitalWrite(LATCH_PIN, LOW);
  // //shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 0);

  // bitSet(led, 2);

  // shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, led);
  // //shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST, 2);


  // digitalWrite(LATCH_PIN, HIGH);

  // delay(1000);

  //uint8_t pinValues[] = { B11101010 };
  //sr.setAll(pinValues);
  sr.set(7, HIGH);

 // delay(1000);

 // uint8_t pinValues2[] = { B00000000 };
  //sr.setAllLow();

  // prender(5, 10);
  // prender(0,0);
  // prender(20,24);
  // prender(0,24);
  // prender(20,0);


  // mostrar(0);  // Mostrar sin scroll
}