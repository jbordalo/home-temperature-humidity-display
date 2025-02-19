#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>
#include <DHT22.h>

#define TFT_CS         D1
#define TFT_RST        D8                                    
#define TFT_DC         D2
#define TFT_MOSI       D5
#define TFT_SCLK       D0

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

#define pinDATA D6

DHT22 dht22(pinDATA); 

void setup(void) {
  Serial.begin(9600);
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextSize(3);
}

float last_t = 0.0;
float last_h = 0.0;

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

void loop() {
  print_measurements(last_t, last_h, ST77XX_BLACK);

  float t = dht22.getTemperature();
  float h = dht22.getHumidity();

  last_t = t;
  last_h = h;

  Serial.print("h=");Serial.print(h,1);Serial.print("\t");
  Serial.print("t=");Serial.println(t,1);

  print_measurements(t, h, ST77XX_WHITE);

  delay(5000);
}
