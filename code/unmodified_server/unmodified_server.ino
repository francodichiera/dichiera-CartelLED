#include <WiFi.h>

// === REMAPEO DE SERIAL PARA LIBERAR GPIO1/GPIO3 ===
#define DEBUG_BAUD 115200
#define DEBUG_RX   16
#define DEBUG_TX   17

// === PINS PARA 74HC595 ===
#define DATA_PIN   23
#define CLOCK_PIN  18
#define LATCH_PIN  19

// === PINS PARA FILAS (TIP122) ===
#define MATRIX_ROWS 20
const uint8_t filas[MATRIX_ROWS] = {
  3, 1, 2, 4, 5,
  12,13,14,15,16,
  17,21,22,25,26,
  27,32,33,34,35
};

// === CREDENCIALES WiFi ===
const char* ssid     = "iPhone Fran";
const char* password = "holaqtal";

// === SERVIDOR HTTP ===
WiFiServer server(80);

// === BUFFERS Y PARÁMETROS GLOBALES ===
#define BUF_SIZE 512
char curMessage[BUF_SIZE] = "";
char newMessage[BUF_SIZE] = "";
bool newMessageAvailable    = false;
bool invertMode             = false;
bool scrollRight            = false;
uint16_t scrollSpeed        = 40;


/* Cuando tengas el código que genera texto como ledMatrix[20][96], 
lo reemplazás directamente.*/

// === MATRIZ SIMULADA DE 20x96 ===
uint8_t ledMatrix[20][96];  // Esto se llenará con 0s y 1s

// === FUNCIONES PARA EL DISPLAY ===
void enviarDatos(uint32_t data) {
  digitalWrite(LATCH_PIN, LOW);
  for (int i = 23; i >= 0; i--) {
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(DATA_PIN, (data & (1UL << i)) ? HIGH : LOW);
    digitalWrite(CLOCK_PIN, HIGH);
  }
  digitalWrite(LATCH_PIN, HIGH);
}

void mostrarFrame(const uint8_t matrix[20][96], int desplazamiento) {
  for (int f = 0; f < 20; f++) {
    uint32_t datos = 0;
    for (int c = 0; c < 24; c++) {
      int col = (desplazamiento + c) % 96;
      datos = (datos << 1) | matrix[f][col];
    }

    digitalWrite(filas[f], LOW);      // Activar fila
    enviarDatos(datos);               // Mostrar columnas
    delayMicroseconds(500);
    digitalWrite(filas[f], HIGH);     // Apagar fila
  }
}

// === A futuro con v0 el front del sv se deja pro (ya intente) ===
// === RESPUESTA Y PÁGINA WEB ===
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

// === UTILIDADES PARA PARSEO ===
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

void setup() {
  Serial.begin(DEBUG_BAUD, SERIAL_8N1, DEBUG_RX, DEBUG_TX);
  Serial.println("=== ESP32 Matriz 20x96 con 74HC595 ===");

  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    digitalWrite(filas[i], HIGH);
  }

  pinMode(DATA_PIN,  OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());
  server.begin();

  // === PATRÓN DE PRUEBA PARA LEDMATRIX ===
  for (int f = 0; f < 20; f++) {
    for (int c = 0; c < 96; c++) {
      ledMatrix[f][c] = (c % 8) < 4 ? 1 : 0; // Rayas verticales
    }
  }
}

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

  // === MOSTRAR MATRIZ SIMULADA ===
  static int desplazamiento = 0;
  static unsigned long lastScroll = 0;
  if (millis() - lastScroll > scrollSpeed) {
    lastScroll = millis();
    desplazamiento = (desplazamiento + 1) % 96;
  }

  mostrarFrame(ledMatrix, desplazamiento);
}
