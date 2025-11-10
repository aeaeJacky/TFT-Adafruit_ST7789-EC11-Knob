#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4

#define DARKGREY 0x7BEF  // Custom RGB565 color: dark grey

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define CENTER_X 160
#define CENTER_Y 120

// --- Forward: Car straight ---
void drawCarLogo_forward() {
  tft.fillScreen(ST77XX_BLACK);

  // Car body
  tft.fillRoundRect(CENTER_X - 50, CENTER_Y - 25, 100, 50, 15, ST77XX_BLUE);
  // Roof
  tft.fillRoundRect(CENTER_X - 30, CENTER_Y - 45, 60, 25, 8, ST77XX_CYAN);
  // Wheels
  tft.fillCircle(CENTER_X - 37, CENTER_Y + 25, 10, DARKGREY);
  tft.fillCircle(CENTER_X + 37, CENTER_Y + 25, 10, DARKGREY);
  // Arrow up
  tft.fillTriangle(CENTER_X, CENTER_Y - 65, CENTER_X - 12, CENTER_Y - 45, CENTER_X + 12, CENTER_Y - 45, ST77XX_GREEN);
  tft.fillRect(CENTER_X - 6, CENTER_Y - 45, 12, 22, ST77XX_GREEN);

  // Label
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("FORWARD", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y + 45);
  tft.print("FORWARD");
}

// --- Left: Car tilted left ---
void drawCarLogo_left() {
  tft.fillScreen(ST77XX_BLACK);

  // Car body (skewed to left)
  tft.fillRoundRect(CENTER_X - 60, CENTER_Y - 10, 100, 50, 15, ST77XX_BLUE);
  // Roof (skewed)
  tft.fillRoundRect(CENTER_X - 45, CENTER_Y - 35, 60, 25, 8, ST77XX_CYAN);
  // Wheels (closer left, farther right)
  tft.fillCircle(CENTER_X - 55, CENTER_Y + 35, 10, DARKGREY);
  tft.fillCircle(CENTER_X + 20, CENTER_Y + 15, 10, DARKGREY);
  // Arrow up-left
  tft.fillTriangle(CENTER_X - 40, CENTER_Y - 45, CENTER_X - 60, CENTER_Y - 25, CENTER_X - 20, CENTER_Y - 25, ST77XX_YELLOW);
  tft.fillRect(CENTER_X - 35, CENTER_Y - 25, 10, 22, ST77XX_YELLOW);

  // Label
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("LEFT", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y + 55);
  tft.print("LEFT");
}

// --- Right: Car tilted right ---
void drawCarLogo_right() {
  tft.fillScreen(ST77XX_BLACK);

  // Car body (skewed to right)
  tft.fillRoundRect(CENTER_X - 40, CENTER_Y - 10, 100, 50, 15, ST77XX_BLUE);
  // Roof (skewed)
  tft.fillRoundRect(CENTER_X - 15, CENTER_Y - 35, 60, 25, 8, ST77XX_CYAN);
  // Wheels (closer right, farther left)
  tft.fillCircle(CENTER_X + 55, CENTER_Y + 35, 10, DARKGREY);
  tft.fillCircle(CENTER_X - 20, CENTER_Y + 15, 10, DARKGREY);
  // Arrow up-right
  tft.fillTriangle(CENTER_X + 40, CENTER_Y - 45, CENTER_X + 60, CENTER_Y - 25, CENTER_X + 20, CENTER_Y - 25, ST77XX_YELLOW);
  tft.fillRect(CENTER_X + 25, CENTER_Y - 25, 10, 22, ST77XX_YELLOW);

  // Label
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("RIGHT", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y + 55);
  tft.print("RIGHT");
}

void setup() {
  tft.init(240, 320, SPI_MODE2);
  tft.setRotation(3); // Landscape
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(50, 120);
  tft.print("Car Status Demo");
  delay(1500);
}

void loop() {
  drawCarLogo_forward();
  delay(2000);

  drawCarLogo_left();
  delay(2000);

  drawCarLogo_right();
  delay(2000);
}