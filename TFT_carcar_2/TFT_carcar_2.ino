#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS    15
#define TFT_DC    2
#define TFT_RST   4

#define DARKGREY 0x7BEF  // Custom RGB565 color: dark grey

#define K0_PIN 5  // CHANGE THIS to your real K0 pin!

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

#define CENTER_X 160
#define CENTER_Y 120

// --- Draw car with flag for ready state ---
void drawReadyLogo() {
  tft.fillScreen(ST77XX_BLACK);

  // Car body (centered)
  tft.fillRoundRect(CENTER_X - 50, CENTER_Y - 10, 100, 40, 12, ST77XX_BLUE);
  // Roof
  tft.fillRoundRect(CENTER_X - 30, CENTER_Y - 25, 60, 20, 6, ST77XX_CYAN);
  // Wheels
  tft.fillCircle(CENTER_X - 32, CENTER_Y + 25, 8, DARKGREY);
  tft.fillCircle(CENTER_X + 32, CENTER_Y + 25, 8, DARKGREY);

  // Fancy "ready" flag (like a race flag)
  tft.fillRect(CENTER_X + 20, CENTER_Y - 35, 4, 20, ST77XX_WHITE); // flag pole
  for (int i = 0; i < 3; i++) {
    tft.fillRect(CENTER_X + 24, CENTER_Y - 35 + i * 6, 18, 3, (i % 2 == 0) ? ST77XX_WHITE : ST77XX_BLACK);
  }

  // READY? text
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(3);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("READY?", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y - 55);
  tft.print("READY?");

  // Press button message
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.getTextBounds("Press K0 to Start", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y + 50);
  tft.print("Press K0 to Start");
}

// --- Draw countdown number ---
void drawCountdown(int n) {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(8);
  int16_t x1, y1;
  uint16_t w, h;
  char buf[2];
  sprintf(buf, "%d", n);
  tft.getTextBounds(buf, 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y - h/2);
  tft.print(buf);
}

// --- Draw START! scenario logo ---
void drawStartLogo() {
  tft.fillScreen(ST77XX_BLACK);

  // Car body (centered, "speed lines" at back)
  for (int i = 0; i < 3; i++) {
    tft.drawLine(CENTER_X - 55 - i*7, CENTER_Y, CENTER_X - 70 - i*10, CENTER_Y - 10 + i*10, ST77XX_WHITE);
  }
  tft.fillRoundRect(CENTER_X - 50, CENTER_Y - 10, 100, 40, 12, ST77XX_RED);
  tft.fillRoundRect(CENTER_X - 30, CENTER_Y - 25, 60, 20, 6, ST77XX_ORANGE);
  tft.fillCircle(CENTER_X - 32, CENTER_Y + 25, 8, DARKGREY);
  tft.fillCircle(CENTER_X + 32, CENTER_Y + 25, 8, DARKGREY);

  // Big "START!" text
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(4);
  int16_t x1, y1; uint16_t w, h;
  tft.getTextBounds("START!", 0, 0, &x1, &y1, &w, &h);
  tft.setCursor(CENTER_X - w/2, CENTER_Y + 40);
  tft.print("START!");
}

void setup() {
  tft.init(240, 320, SPI_MODE2);
  tft.setRotation(3); // Landscape

  pinMode(K0_PIN, INPUT_PULLUP); // Button default HIGH, pressed LOW
}

void loop() {
  // --- READY STATE ---
  drawReadyLogo();
  // Wait for K0 press
  while (digitalRead(K0_PIN) == HIGH) {
    delay(10);
  }
  delay(100); // debounce

  // --- COUNTDOWN ---
  for (int i = 3; i >= 1; i--) {
    drawCountdown(i);
    delay(1000);
  }

  // --- START STATE ---
  drawStartLogo();
  delay(1500);

  // --- AUTONOMOUS CODE PLACEHOLDER ---
  // Replace this loop with your autonomous/human-follow code
  while (1) {
    // Draw the start logo forever (or you can transition to your next code)
    drawStartLogo();
    delay(500);
  }
}