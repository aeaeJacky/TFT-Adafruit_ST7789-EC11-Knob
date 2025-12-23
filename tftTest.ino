#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// ---------- TFT (320x240) ----------
#define TFT_CS   10
#define TFT_DC    5
#define TFT_RST   4
// MOSI = 11, SCK = 13 (hardware SPI)

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// ---------- INPUTS ----------
const int encA   = 2;   // encoder A on D2 (INT0)
const int encB   = 3;   // encoder B on D3 (INT1)
const int encBtn = A2;  // encoder push button
const int btn1   = A3;  // extra button

volatile long encoderCount = 0;  // must be volatile for ISR
volatile int lastEncoded = 0;

int  prevA, prevB;
bool prevEncPressed, prevB1Pressed;
long prevCount;
unsigned long lastUpdate = 0;

// ISR for encoder (called on any change on A or B)
void updateEncoder() {
  int MSB = digitalRead(encA); // most significant bit
  int LSB = digitalRead(encB); // least significant bit

  int encoded = (MSB << 1) | LSB;       // 2-bit value
  int sum = (lastEncoded << 2) | encoded; 

  // These values correspond to valid steps
  if (sum == 0b0001 || sum == 0b0111 || sum == 0b1110 || sum == 0b1000) {
    encoderCount++;
  } else if (sum == 0b0010 || sum == 0b0100 || sum == 0b1101 || sum == 0b1011) {
    encoderCount--;
  }

  lastEncoded = encoded;
}

void setup() {
  Serial.begin(115200);

  pinMode(encA,   INPUT_PULLUP);
  pinMode(encB,   INPUT_PULLUP);
  pinMode(encBtn, INPUT_PULLUP);
  pinMode(btn1,   INPUT_PULLUP);

  // Initialize lastEncoded for ISR
  int MSB = digitalRead(encA);
  int LSB = digitalRead(encB);
  lastEncoded = (MSB << 1) | LSB;

  // Attach interrupts on pin 2 and 3
  attachInterrupt(digitalPinToInterrupt(encA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encB), updateEncoder, CHANGE);

  // For 2.4" 320x240 ST7789
  tft.init(240, 320);   // width=240, height=320
  tft.setRotation(3);   // landscape: 320x240
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextWrap(false);

  // Header drawn once
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.println("Inputs Test");
  tft.setTextSize(1);
  tft.println("EncA=D2, EncB=D3");
  tft.println("EncBtn=A2, Btn1=A3");
  tft.println("Turn encoder & press");

  // Force first update
  prevA = -1;
  prevB = -1;
  prevEncPressed = prevB1Pressed = false;
  prevCount = 123456789;
}

void printLine(uint16_t x, uint16_t y, const char* label, const char* value) {
  tft.setCursor(x, y);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK); // text + background
  tft.print(label);
  tft.print(value);
}

void loop() {
  // ----- Read A/B for display only -----
  int a = digitalRead(encA);
  int b = digitalRead(encB);

  bool encPressed = (digitalRead(encBtn) == LOW);
  bool b1Pressed  = (digitalRead(btn1)   == LOW);

  // Take a snapshot of encoderCount safely
  noInterrupts();
  long countSnapshot = encoderCount;
  interrupts();

  // ----- Serial debug -----
  Serial.print("A=");      Serial.print(a);
  Serial.print(" B=");     Serial.print(b);
  Serial.print(" EncBtn=");Serial.print(encPressed);
  Serial.print(" Btn1=");  Serial.print(b1Pressed);
  Serial.print(" Count="); Serial.println(countSnapshot);

  // ----- Decide if we need to redraw -----
  bool changed = (a != prevA) ||
                 (b != prevB) ||
                 (encPressed != prevEncPressed) ||
                 (b1Pressed  != prevB1Pressed)  ||
                 (countSnapshot != prevCount);

  unsigned long now = millis();
  if (changed || now - lastUpdate > 100) {  // limit update rate
    lastUpdate = now;

    char buf[16];

    // A
    snprintf(buf, sizeof(buf), "%d  ", a);
    printLine(0, 60,  "A: ", buf);

    // B
    snprintf(buf, sizeof(buf), "%d  ", b);
    printLine(0, 85,  "B: ", buf);

    // EncBtn
    printLine(0, 110, "EncBtn: ", encPressed ? "PRESSED   " : "RELEASE   ");

    // Btn1
    printLine(0, 135, "Btn1: ",   b1Pressed  ? "PRESSED   " : "RELEASE   ");

    // Count
    snprintf(buf, sizeof(buf), "%ld    ", countSnapshot);
    printLine(0, 160, "Count: ", buf);

    // Save current as previous
    prevA = a;
    prevB = b;
    prevEncPressed = encPressed;
    prevB1Pressed  = b1Pressed;
    prevCount      = countSnapshot;
  }

  delay(10);
}