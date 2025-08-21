#include <Arduino.h>
#include <ShiftRegister74HC595.h> // Librería para controlar registros de desplazamiento 74HC595
#include <Adafruit_GFX.h>         // Librería gráfica base de Adafruit
#include <Fonts/FreeSans12pt7b.h>  // Fuente 9 puntos (tipo sans serif)

// === CONFIGURACIÓN DE PINES ===
#define DATA_PIN 23   // Pin de datos (DS) hacia el primer 74HC595
#define CLOCK_PIN 18  // Pin de reloj (SHCP) de los 74HC595
#define LATCH_PIN 19  // Pin de latch (STCP) de los 74HC595

// === CONFIGURACIÓN DE MATRIZ LED ===
#define MATRIX_ROWS 18               // Cantidad de filas físicas en la matriz
#define MATRIX_COLS 24              // Cantidad de columnas físicas en la matriz
#define BYTES_PER_ROW (MATRIX_COLS / 8)  // Cada fila se representa en bytes (3 bytes para 24 columnas)

// Creamos instancia para manejar 3 registros 74HC595 (cada uno controla 8 columnas)
ShiftRegister74HC595<3> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);

// Pines que controlan las 18 filas a través de transistores TIP122
const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12, 13, 14, 15, 16,
  17, 21, 22, 25, 26,
  27, 32, 33
};

// Framebuffer: guarda el estado (encendido/apagado) de cada LED de la matriz.
// Cada bit representa un LED.
uint8_t framebuffer[MATRIX_ROWS][BYTES_PER_ROW];

// Clase personalizada que hereda de Adafruit_GFX para dibujar en el framebuffer
class MatrixDisplay : public Adafruit_GFX {
public:
  MatrixDisplay() : Adafruit_GFX(MATRIX_COLS, MATRIX_ROWS) {}

  // Dibuja un pixel en las coordenadas (x, y) dentro del framebuffer
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= MATRIX_COLS || y < 0 || y >= MATRIX_ROWS) return;

    uint8_t bitMask = 0x80 >> (x % 8);      // Máscara para el bit que corresponde a la columna
    uint8_t &byte = framebuffer[y][x / 8];  // Referencia al byte donde está el pixel

    if (color) byte |= bitMask;   // Enciende el LED
    else byte &= ~bitMask;        // Apaga el LED
  }

  // Borra todo el contenido del framebuffer (apaga toda la matriz)
  void clear() {
    for (int i = 0; i < MATRIX_ROWS; i++)
      for (int j = 0; j < BYTES_PER_ROW; j++)
        framebuffer[i][j] = 0;
  }
};

MatrixDisplay display; // Instancia global para manejar la matriz

// === CONTROL DE FILAS ===
void prenderFila(uint8_t fila) { digitalWrite(filas[fila], HIGH); }
void apagarFila(uint8_t fila) { digitalWrite(filas[fila], LOW); }

// === FUNCIÓN DE REFRESCO DE MATRIZ ===
// Multiplexa todas las filas rápidamente para que el ojo las perciba encendidas al mismo tiempo
void refrescarMatrizCompleta() {
  for (uint8_t fila = 0; fila < MATRIX_ROWS; fila++) {
    // Apaga la fila anterior para evitar "ghosting" (efecto de LEDs encendidos donde no corresponde)
    apagarFila((fila + MATRIX_ROWS - 1) % MATRIX_ROWS);

    // Carga el patrón de LEDs para esta fila en los registros de desplazamiento
    for (uint8_t bit = 0; bit < MATRIX_COLS; bit++) {
      uint8_t colByte = bit / 8;             // Qué byte del framebuffer contiene este bit
      uint8_t bitMask = 0x80 >> (bit % 8);   // Qué bit dentro de ese byte es
      bool ledOn = (framebuffer[fila][colByte] & bitMask) != 0;
      sr.set(bit, ledOn ? HIGH : LOW);
    }

    // Enciende la fila actual para mostrarla
    prenderFila(fila);

    // Tiempo que la fila permanece encendida (controla brillo y parpadeo)
    delayMicroseconds(200);

    // Apaga la fila antes de pasar a la siguiente
    apagarFila(fila);
  }
}

// === TEXTO A MOSTRAR ===
const char *mensaje = "BIENVENIDOS ELECTRONICA EXPO HUERGO 2025";
int16_t x = MATRIX_COLS;  // Posición inicial del texto (empieza fuera de la pantalla, a la derecha)

void setup() {
  // Configuración inicial de pines
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    apagarFila(i);
  }
  sr.setAllLow(); // Apaga todas las columnas al inicio

  // Configuración de la pantalla virtual
  display.setFont(&FreeSans12pt7b); // Selecciona fuente
  display.setTextColor(1);         // "Color" 1 = encendido
  display.setTextWrap(false);      // Sin salto automático de línea
}

void loop() {
  static unsigned long lastScroll = 0;

  // Solo actualizamos el texto cada 75 ms para el efecto de scroll suave
  if (millis() - lastScroll > 100) {
    lastScroll = millis();

    display.clear();          // Limpia framebuffer
    display.setCursor(x, 17); // Posición del texto (X variable para scroll, Y ajustada para altura de fuente)
    display.print(mensaje);   // Escribe el mensaje en el framebuffer

    // Calcula ancho del texto para saber cuándo reiniciar scroll
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(mensaje, 0, 0, &x1, &y1, &w, &h);

    x--; // Desplaza texto 1 píxel a la izquierda

    // Si el texto salió completamente de la pantalla, lo reiniciamos a la derecha
    if (x < -w) {
      x = MATRIX_COLS;
    }
  }

  // Refrescamos la matriz en cada ciclo para evitar parpadeo
  refrescarMatrizCompleta();
}
