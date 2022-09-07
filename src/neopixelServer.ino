// Neopixel strip web control
// Pablo Sebastián Areas Cárcano
//
// reference,
// https://RandomNerdTutorials.com/esp8266-nodemcu-web-server-websocket-sliders/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <ezButton.h>
ezButton botonON(D5);
ezButton botonMODO(D6);
int encendido = 0, modo = 1; //modo de 1 a 5

#include <Adafruit_NeoPixel.h>
#define LED_PIN D1 // salida de datos
#define LED_COUNT 60 //cantidad de leds
#define BRIGHTNESS 255 //brillo maximo, max 255
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

int r = 0, g = 0, b = 0, w = 0;
int posicion = 27, pos_min = 0, pos_max = 0, ancho = 54;

// Replace with your network credentials
const char* ssid = "yourssid";
const char* password = "yourpassword";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

String message = "";
String sliderValue1 = "0"; //r
String sliderValue2 = "0"; //g
String sliderValue3 = "0"; //b
String sliderValue4 = "0"; //w
String sliderValue5 = "50";  //posicion
String sliderValue6 = "100";  //ancho

//Json Variable to Hold Slider Values
JSONVar sliderValues;

//Get Slider Values
String getSliderValues() {
  sliderValues["sliderValue1"] = String(sliderValue1);
  sliderValues["sliderValue2"] = String(sliderValue2);
  sliderValues["sliderValue3"] = String(sliderValue3);
  sliderValues["sliderValue4"] = String(sliderValue4);
  sliderValues["sliderValue5"] = String(sliderValue5);
  sliderValues["sliderValue6"] = String(sliderValue6);

  String jsonString = JSON.stringify(sliderValues);
  return jsonString;
}

// Initialize LittleFS
void initFS() {
  if (!LittleFS.begin()) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  else {
    Serial.println("LittleFS mounted successfully");
  }
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void actualiza_tira() {
  strip.clear();

  pos_min = posicion - ancho;
  pos_max = posicion + ancho;

  for (int i = pos_min; i < pos_max; i++) {
    strip.setPixelColor(i, r, g, b, w);
  }

  strip.show();
}

void muestra_blanco() {
  r = 0; sliderValue1 = "0";
  g = 0; sliderValue2 = "0";
  b = 0; sliderValue3 = "0";
  w = 255; sliderValue4 = "100";
  posicion = LED_COUNT / 2; sliderValue5 = "50";
  ancho = LED_COUNT; sliderValue6 = "100";

  actualiza_tira();
  notifyClients(getSliderValues());
}

void muestra_rojo() {
  r = 255; sliderValue1 = "100";
  g = 0; sliderValue2 = "0";
  b = 0; sliderValue3 = "0";
  w = 0; sliderValue4 = "0";
  posicion = LED_COUNT / 2; sliderValue5 = "50";
  ancho = LED_COUNT; sliderValue6 = "100";

  actualiza_tira();
  notifyClients(getSliderValues());
}

void muestra_verde() {
  r = 0; sliderValue1 = "0";
  g = 255; sliderValue2 = "100";
  b = 0; sliderValue3 = "0";
  w = 0; sliderValue4 = "0";
  posicion = LED_COUNT / 2; sliderValue5 = "50";
  ancho = LED_COUNT; sliderValue6 = "100";

  actualiza_tira();
  notifyClients(getSliderValues());
}

void muestra_azul() {
  r = 0; sliderValue1 = "0";
  g = 0; sliderValue2 = "0";
  b = 255; sliderValue3 = "100";
  w = 0; sliderValue4 = "0";
  posicion = LED_COUNT / 2; sliderValue5 = "50";
  ancho = LED_COUNT; sliderValue6 = "100";

  actualiza_tira();
  notifyClients(getSliderValues());
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void muestra_rainbow() {
  uint16_t i, j;
  for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(20);
  }
}

void muestra_rainbow_static() {
  uint16_t i;

  for (i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels())) & 255));
    strip.show();
  }
}

void apaga() {
  r = 0; sliderValue1 = "0";
  g = 0; sliderValue2 = "0";
  b = 0; sliderValue3 = "0";
  w = 0; sliderValue4 = "0";
  posicion = LED_COUNT / 2; sliderValue5 = "50";
  ancho = LED_COUNT; sliderValue6 = "100";

  actualiza_tira();
  notifyClients(getSliderValues());
}

void notifyClients(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (message.indexOf("1s") >= 0) {
      sliderValue1 = message.substring(2);
      r = map(sliderValue1.toInt(), 0, 100, 0, 255);
      actualiza_tira();
      Serial.println(r);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("2s") >= 0) {
      sliderValue2 = message.substring(2);
      g = map(sliderValue2.toInt(), 0, 100, 0, 255);
      actualiza_tira();
      Serial.println(g);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("3s") >= 0) {
      sliderValue3 = message.substring(2);
      b = map(sliderValue3.toInt(), 0, 100, 0, 255);
      actualiza_tira();
      Serial.println(b);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("4s") >= 0) {
      sliderValue4 = message.substring(2);
      w = map(sliderValue4.toInt(), 0, 100, 0, 255);
      actualiza_tira();
      Serial.println(w);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("5s") >= 0) {
      sliderValue5 = message.substring(2);
      posicion = map(sliderValue5.toInt(), 0, 100, 0, 60);
      actualiza_tira();
      Serial.println(posicion);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("6s") >= 0) {
      sliderValue6 = message.substring(2);
      ancho = map(sliderValue6.toInt(), 0, 100, 0, 30);
      actualiza_tira();
      Serial.println(ancho);
      Serial.print(getSliderValues());
      notifyClients(getSliderValues());
    }
    if (message.indexOf("rain") >= 0) {
      muestra_rainbow_static();
    }
    if (message.indexOf("apaga") >= 0) {
      apaga();
    }
    if (strcmp((char*)data, "getValues") == 0) {
      notifyClients(getSliderValues());
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() {
  Serial.begin(115200);

  botonON.setDebounceTime(50); //50ms
  botonMODO.setDebounceTime(50);

  strip.begin(); //inicializa tira
  strip.show(); //apaga tira
  strip.setBrightness(BRIGHTNESS);

  initFS();
  initWiFi();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  ws.cleanupClients();

  botonON.loop();
  botonMODO.loop();

  if (botonON.isPressed()) {
    encendido = !encendido;
    if (encendido)
      muestra_blanco();
    else {
      apaga();
      modo = 1;
    }
  }

  if (botonMODO.isPressed()) {
    if (encendido) {
      modo++;
      if (modo == 6)
        modo = 1;

      switch (modo) {
        case 1:
          muestra_blanco();
          break;
        case 2:
          muestra_rojo();
          break;
        case 3:
          muestra_verde();
          break;
        case 4:
          muestra_azul();
          break;
        case 5:
          muestra_rainbow();
          break;
        default:
          break;
      }
    }
  }
}
