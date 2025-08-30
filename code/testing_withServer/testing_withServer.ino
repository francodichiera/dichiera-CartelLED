#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ShiftRegister74HC595.h>
#include <Adafruit_GFX.h>
#include <Fonts/LEMONMILK_Regular10pt7b.h> //https://rop.nl/truetype2gfx/ FONTS PAGE
#include <Preferences.h>

// === CREDENCIALES WiFi ===
const char *ssid = "MovistarFibra-4378A0";  // Nombre de la red WiFi
const char *password = "12345678";          // Contraseña de la red WiFi

// === PINES DE LA MATRIZ ===
#define DATA_PIN 23                      // Pin de datos para 74HC595
#define CLOCK_PIN 18                     // Pin de clock para 74HC595
#define LATCH_PIN 19                     // Pin de latch para 74HC595
#define MATRIX_ROWS 18                   // Cantidad de filas de la matriz de LEDs
#define MATRIX_COLS 24                   // Cantidad de columnas de la matriz de LEDs
#define BYTES_PER_ROW (MATRIX_COLS / 8)  // Cantidad de bytes por fila (24/8=3)

// === CONTROL DE REGISTROS 74HC595 ===
ShiftRegister74HC595<3> sr(DATA_PIN, CLOCK_PIN, LATCH_PIN);  // 3 registros en cascada para 24 columnas

const uint8_t filas[MATRIX_ROWS] = { 3, 1, 2, 4, 5, 12, 13, 14, 15, 16, 17, 21, 22, 25, 26, 27, 32, 33 };
// Pines del ESP32 conectados a las filas (a través de TIP122 o transistores)

uint8_t framebuffer[MATRIX_ROWS][BYTES_PER_ROW];  // Framebuffer donde se almacena qué LEDs deben estar encendidos

// === CLASE PARA EL DISPLAY ===
class MatrixDisplay : public Adafruit_GFX {
public:
  MatrixDisplay()
    : Adafruit_GFX(MATRIX_COLS, MATRIX_ROWS) {}

  // Dibuja un píxel en el framebuffer
  void drawPixel(int16_t x, int16_t y, uint16_t color) override {
    if (x < 0 || x >= MATRIX_COLS || y < 0 || y >= MATRIX_ROWS) return;
    uint8_t bitMask = 0x80 >> (x % 8);      // Máscara para elegir el bit correcto
    uint8_t &byte = framebuffer[y][x / 8];  // Referencia al byte donde está ese LED
    if (color) byte |= bitMask;             // Si color != 0 → prende el LED
    else byte &= ~bitMask;                  // Si color == 0 → apaga el LED
  }

  // Limpia toda la pantalla
  void clear() {
    for (int i = 0; i < MATRIX_ROWS; i++)
      for (int j = 0; j < BYTES_PER_ROW; j++) framebuffer[i][j] = 0;
  }
};
MatrixDisplay display;  // Instancia global del display

// === FUNCIONES PARA CONTROLAR FILAS ===
void prenderFila(uint8_t f) {
  digitalWrite(filas[f], HIGH);  // Activa la fila f
}
void apagarFila(uint8_t f) {
  digitalWrite(filas[f], LOW);  // Desactiva la fila f
}

// === MULTIPLEXADO DE LA MATRIZ ===
void refrescarMatrizCompleta() {
  // Recorre todas las filas una por una
  for (uint8_t f = 0; f < MATRIX_ROWS; f++) {
    // Apaga la fila anterior para evitar "fantasmas"
    apagarFila((f + MATRIX_ROWS - 1) % MATRIX_ROWS);

    // Envía los datos de columnas al registro 74HC595
    for (uint8_t b = 0; b < MATRIX_COLS; b++) {
      uint8_t byteCol = b / 8;            // Qué byte corresponde a la columna
      uint8_t bitMask = 0x80 >> (b % 8);  // Máscara para ese bit
      sr.set(b, (framebuffer[f][byteCol] & bitMask) ? HIGH : LOW);
    }

    prenderFila(f);          // Enciende la fila actual
    delayMicroseconds(200);  // Pequeña pausa para que se vea
    apagarFila(f);           // Apaga la fila antes de la siguiente
  }
}

// === SERVIDOR HTTP ===
WiFiServer server(80);  // Servidor web en el puerto 80

#define BUF_SIZE 2048
char curMessage[BUF_SIZE] = "";    // Mensaje actual que se muestra
char newMessage[BUF_SIZE] = "";    // Nuevo mensaje recibido
bool newMessageAvailable = false;  // Flag para indicar si hay un nuevo mensaje
bool scrollRight = false;          // Dirección de scroll (false = izquierda, true = derecha)
uint16_t scrollSpeed = 100;        // Velocidad del scroll

// Preferences: guarda datos en memoria flash para que persistan tras reinicio
Preferences prefs;

// === PÁGINA WEB QUE MANEJA EL CARTEL ===
const char WebResponse[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n";
const char WebPage[] =
  "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1.0,maximum-scale=1.0,user-scalable=no'><title>CARTEL INTELIGENTE HUERGO</title><link href='https://fonts.googleapis.com/css2?family=Poppins:wght@400;600&family=DotGothic16&display=swap' rel='stylesheet'><style>body{font-family:'Poppins',sans-serif;background:#f3f0ff;color:#2a1a66;text-align:center;margin:0;padding:0}h1{background:linear-gradient(90deg,#3399ff,#9933ff);color:#fff;padding:15px 0;margin:0;font-size:1.5em;letter-spacing:1px}form{padding:20px;background:linear-gradient(145deg,#e6e0ff,#fff);margin:20px auto;width:90%;max-width:420px;border-radius:15px;box-shadow:0 8px 20px rgba(0,0,0,.15);text-align:center;transition:.3s}form:hover{box-shadow:0 10px 25px rgba(0,0,0,.25)}input[type=text]{width:95%;font-size:16px;padding:10px;margin:10px 0;border:2px solid #3399ff;border-radius:8px;transition:.3s;outline:none;font-family:'Poppins',sans-serif}input[type=text]:focus{border-color:#9933ff;box-shadow:0 0 8px rgba(153,51,255,.4)}input[type=range]{-webkit-appearance:none;width:95%;height:10px;border-radius:5px;background:#ccc;outline:none}input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border-radius:50%;background:#3399ff;cursor:pointer;box-shadow:0 2px 6px rgba(0,0,0,.3);transition:.3s}input[type=range]::-webkit-slider-thumb:hover{background:#9933ff}button{padding:12px 25px;margin-top:10px;background:linear-gradient(90deg,#3399ff,#9933ff);color:#fff;border:none;border-radius:8px;cursor:pointer;box-shadow:0 4px 12px rgba(0,0,0,.2);transition:.3s;font-size:1em;font-family:'Poppins',sans-serif}button:hover{background:linear-gradient(90deg,#0055aa,#6600cc);box-shadow:0 6px 15px rgba(0,0,0,.3)}#speedVal{font-weight:bold;color:#2a1a66;display:inline-block;width:50px;text-align:center;font-family:'Poppins',sans-serif}.demoScroll{margin:25px auto;padding:10px;width:90%;max-width:420px;background:#000;border:2px solid #3399ff;border-radius:10px;overflow:hidden;height:50px;display:flex;align-items:center;position:relative}.ledText{color:#0af;font-family:'DotGothic16',monospace;font-size:1.6em;font-weight:bold;letter-spacing:2px;text-shadow:none;white-space:nowrap;position:absolute;will-change:transform}footer{margin-top:30px;color:#9933ff;font-size:.9em;font-family:'Poppins',sans-serif}@media(max-width:500px){input[type=text],input[type=range]{width:95%}}</style><script>let animId=null;function SendData(){var msg=encodeURIComponent(document.frm.Message.value);var SD=document.frm.ScrollType.value;var SP=document.frm.Speed.value;window.location='/?MSG='+msg+'&SD='+SD+'&SP='+SP}function updateSpeed(val){document.getElementById('speedVal').innerText=val;updateDemo()}function updateDemo(){const txt=document.frm.Message.value||'Vista previa del mensaje';const dir=document.frm.ScrollType.value;const spd=document.frm.Speed.value;const el=document.getElementById('demoText');el.textContent=txt;if(animId)cancelAnimationFrame(animId);const parent=el.parentNode;let pos=(dir==='L')?parent.offsetWidth:-el.offsetWidth;function step(){let velocidad=(210-spd)/20;pos+=(dir==='L'?-1:1)*velocidad;if((dir==='L'&&pos<-el.offsetWidth)||(dir==='R'&&pos>parent.offsetWidth)){pos=(dir==='L')?parent.offsetWidth:-el.offsetWidth}el.style.transform='translateX('+pos+'px)';animId=requestAnimationFrame(step)}step()}window.onload=updateDemo;</script></head><body><h1>CARTEL INTELIGENTE HUERGO</h1><form name='frm'>Mensaje:<br><input type='text' name='Message' maxlength='255' oninput='updateDemo()'><br><br><input type='radio' name='ScrollType' value='L' checked onclick='updateDemo()'>Izquierda<input type='radio' name='ScrollType' value='R' onclick='updateDemo()'>Derecha<br><br>Velocidad: <span id='speedVal'>100</span><br><input type='range' name='Speed' min='10' max='200' value='100' oninput='updateSpeed(this.value)'><br><br></form><div class='demoScroll'><div id='demoText' class='ledText'>Vista previa del mensaje</div></div><button onclick='SendData()'>Enviar</button><footer>Realizado por FRANCO DICHIERA 2025</footer></body></html>";


// === FUNCIONES DE UTILIDAD ===
// Convierte un carácter hexadecimal a número
uint8_t htoi(char c) {
  c = toupper(c);
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return 0;
}

// Parsea la URL recibida para extraer mensaje, dirección y velocidad
void getData(char *req) {
  char *p;
  // Buscar mensaje (MSG=...)
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
  // Buscar dirección (SD=L o R)
  p = strstr(req, "SD=");
  if (p) scrollRight = (*(p + 3) == 'R');
  // Buscar velocidad (SP=xxx)
  p = strstr(req, "SP=");
  if (p) scrollSpeed = atoi(p + 3);
}

// === VARIABLES DE SCROLL ===
int16_t xPos = MATRIX_COLS;  // Posición horizontal del texto

// === SETUP ===
void setup() {
  Serial.begin(115200);

  // Configurar pines de filas como salida
  for (int i = 0; i < MATRIX_ROWS; i++) {
    pinMode(filas[i], OUTPUT);
    apagarFila(i);
  }
  sr.setAllLow();

  // Configurar display
  display.setFont(&LEMONMILK_Regular10pt7b);  // Fuente
  display.setTextColor(1);           // Color blanco (LED encendido)
  display.setTextWrap(false);        // No cortar líneas

  // Conexión WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado. IP: " + WiFi.localIP().toString());

  // Iniciar mDNS → acceso con "cartelhuergo.local"
  if (MDNS.begin("cartelhuergo")) Serial.println("mDNS iniciado: cartelhuergo.local");
  else Serial.println("Error iniciando mDNS");

  server.begin();  // Inicia servidor HTTP

  // Cargar mensaje y configuraciones guardadas en Preferences
  prefs.begin("cartel", false);
  String savedMsg = prefs.getString("mensaje", "BIENVENIDOS ELECTRONICA EXPO HUERGO 2025");
  savedMsg.toCharArray(curMessage, BUF_SIZE);
  scrollSpeed = prefs.getUInt("speed", 100);
  scrollRight = prefs.getBool("dir", false);
}

// === LOOP PRINCIPAL ===
void loop() {
  // === ATENDER SERVIDOR WEB ===
  WiFiClient client = server.available();
  static char req[BUF_SIZE];
  static int idx = 0;

  if (client) {
    while (client.connected() && client.available()) {
      char c = client.read();
      if (idx < BUF_SIZE - 1) req[idx++] = c;

      // Fin de la cabecera HTTP
      if (c == '\n' && strstr(req, "\r\n\r\n")) {
        req[idx] = '\0';
        getData(req);  // Extraer parámetros de la petición
        client.print(WebResponse);
        client.print(WebPage);
        client.stop();

        // Si hay nuevo mensaje, reemplazar y guardar
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

  // === ANIMACIÓN DE SCROLL ===
  static unsigned long lastScroll = 0;
  if (millis() - lastScroll > scrollSpeed) {
    lastScroll = millis();
    xPos += scrollRight ? 1 : -1;  // Mover texto según dirección

    // Obtener ancho del texto
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(curMessage, 0, 0, &x1, &y1, &w, &h);

    // Resetear posición cuando sale de pantalla
    if (!scrollRight && xPos < -w) xPos = MATRIX_COLS;
    if (scrollRight && xPos > MATRIX_COLS) xPos = -w;
  }

  // Dibujar mensaje en el framebuffer
  display.clear();
  display.setCursor(xPos, 17);  // Posición vertical fija
  display.print(curMessage);

  // Refrescar la matriz LED
  refrescarMatrizCompleta();
}
