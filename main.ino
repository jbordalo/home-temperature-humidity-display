#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <DHT22.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "secrets.h"

#define TFT_CS         D1
#define TFT_RST        D8                                    
#define TFT_DC         D2
#define TFT_MOSI       D5
#define TFT_SCLK       D0

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define pinDATA D6

DHT22 dht22(pinDATA);

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

IPAddress local_IP(192, 168, 1, 100);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);

void handleMetrics() {
  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    server.send(500, "text/plain", "Sensor read error");
    return;
  }

  String metrics = "";

  String clean = server.arg("clean");
  if (clean == "true") {
    // Send human-readable metrics
    metrics = String("Temperature: ") + String(t) + " ºC\n" + 
              String("Humidity: ") + String(h) + " %\n";
  } else {
    // Send Prometheus-formatted metrics
    metrics = String("# HELP esp_temperature_c Temperature in Celsius\n") +
              String("# TYPE esp_temperature_c gauge\n") +
              String("esp_temperature_c{location=\"office\"} ") + String(t) + "\n" +
              String("# HELP esp_humidity_percent Humidity %\n") +
              String("# TYPE esp_humidity_percent gauge\n") +
              String("esp_humidity_percent{location=\"office\"} ") + String(h) + "\n";
  }

  // Return the metrics in Prometheus format
  server.send(200, "text/plain; charset=utf-8", metrics);
}

void handleVersion() {
  server.send(200, "text/plain", "ESP8266 Sensor v1.0.0");
}

void setup(void) {
  Serial.begin(9600);

  // Setup Wifi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  Serial.println("Wifi is on.");

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextSize(3);

  server.on("/version", handleVersion);
  server.on("/metrics", handleMetrics);
  server.begin();

  Serial.println("Web server is on.");
}

void print_measurements(float t, float h, int color) {
  tft.setCursor(10, 30);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(t, 2);
  int save_cursor_y = tft.getCursorY();
  tft.setCursor(tft.getCursorX()+10, tft.getCursorY()-4);
  tft.setTextSize(2);
  tft.print("o");
  tft.setCursor(tft.getCursorX(), save_cursor_y);
  tft.setTextSize(3);
  tft.print("C");
  tft.setCursor(10, 60);
  tft.print(h, 2);
  tft.print(" %");
}

unsigned long lastSensorRead = 0;
const unsigned long SENSOR_INTERVAL = 30000;
const float TEMP_THRESHOLD = 0.2;
const float HUM_THRESHOLD = 1.0;

float last_t = 0.0;
float last_h = 0.0;

void loop() {
  server.handleClient();

  unsigned long now = millis();

  // If not enough time passed for a new reading return
  if (now - lastSensorRead < SENSOR_INTERVAL) {
    return;
  }

  lastSensorRead = now;

  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  if (isnan(t) || isnan(h)) return;

  // Check for threshold
  if (abs(t - last_t) < TEMP_THRESHOLD && abs(h - last_h) < HUM_THRESHOLD) {
    return;
  }

  // This one clears the display
  print_measurements(last_t, last_h, ST77XX_BLACK);

  last_t = t;
  last_h = h;

  Serial.print("h=");Serial.print(h,2);Serial.print("\t");
  Serial.print("t=");Serial.println(t,2);

  print_measurements(t, h, ST77XX_WHITE);

}
