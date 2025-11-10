#include <Arduino.h>
#include <ESP_Knob.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_I2CDevice.h>
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// ----------------------
// Hardware Pin Definitions
// ----------------------
#define GPIO_NUM_KNOB_PIN_A   19
#define GPIO_NUM_KNOB_PIN_B   21

// Button pins: 
// - The encoder button (attached to GPIO22) is used as Enter.
// - The additional button K0 (attached to GPIO5) is used as Back.
#define ENTER_BUTTON_PIN      22
#define BACK_BUTTON_PIN       5

// TFT (ST7789) settings and pins
#define TFT_MOSI              23  // MOSI (Data)
#define TFT_SCLK              18  // Clock (SCLK)
#define TFT_CS                15  // Chip Select
#define TFT_DC                2   // Data/Command
#define TFT_RST               4   // Reset

// ----------------------
// Global Objects and Variables
// ----------------------
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
ESP_Knob *knob;

enum MenuState { MAIN_MENU, EDIT_TIME, EDIT_WEEKDAY, CLOCK_RUNNING };
MenuState currentState = MAIN_MENU;
int menuIndex = 0; // 0: Set Time, 1: Set Weekday, 2: Start Clock

// Variables for time editing & simulation
int editTimeField = 0;  // 0: editing hours, 1: editing minutes
int hourVal   = 12;     // default hour
int minuteVal = 0;      // default minutes
int secondVal = 0;      // used for clock simulation

// Variables for weekday editing
int weekdayVal = 1;       
const char *weekDays[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Button debounce variables
unsigned long lastEnterButtonPressTime = 0;
unsigned long lastBackButtonPressTime  = 0;
const unsigned long buttonDebounceDelay = 200; // ms debounce

int lastEnterButtonState = HIGH;
int lastBackButtonState  = HIGH;

// Clock simulation update timing
unsigned long previousClockMillis = 0;

// Rotary encoder event debounce
const unsigned long encoderDebounceDelay = 50; // ms

// ----------------------
// Encoder Sensitivity Settings
// ----------------------
// For main menu navigation, a higher threshold means you have to turn further to move the cursor.
// For editing detailed settings, use a lower threshold to allow faster adjustments.
const int MENU_ENCODER_THRESHOLD = 4; // Main menu: less sensitive (more steps required)
const int EDIT_ENCODER_THRESHOLD = 2; // Editing: more sensitive (fewer steps required)
int encoderAccumulator = 0;

// Global variable to hold the cumulative raw value from the EC11 encoder.
volatile int encoderRawValue = 0;

// ----------------------
// Forward Declarations
// ----------------------
void drawMainMenu();
void drawEditTime();
void drawEditWeekday();
void drawClock();

void handleEnterButtonPress();
void handleBackButtonPress();

// This function processes the accumulated encoder events. It uses a different threshold
// depending on whether we are in the main menu or editing a setting.
void processEncoderInput(int d) {
  encoderAccumulator += d;
  int currentThreshold = (currentState == MAIN_MENU ? MENU_ENCODER_THRESHOLD : EDIT_ENCODER_THRESHOLD);
  
  if (encoderAccumulator >= currentThreshold) {
    // Process a positive step.
    switch (currentState) {
      case MAIN_MENU:
        menuIndex++;
        if (menuIndex > 2) menuIndex = 0;
        drawMainMenu();
        break;
      case EDIT_TIME:
        if (editTimeField == 0) {  // Adjust hours
          hourVal++;
          if (hourVal > 23) hourVal = 0;
        } else {                   // Adjust minutes
          minuteVal++;
          if (minuteVal > 59) minuteVal = 0;
        }
        drawEditTime();
        break;
      case EDIT_WEEKDAY:
        weekdayVal++;
        if (weekdayVal > 6) weekdayVal = 0;
        drawEditWeekday();
        break;
      default:
        break;
    }
    encoderAccumulator -= currentThreshold;
  }
  else if (encoderAccumulator <= -currentThreshold) {
    // Process a negative step.
    switch (currentState) {
      case MAIN_MENU:
        menuIndex--;
        if (menuIndex < 0) menuIndex = 2;
        drawMainMenu();
        break;
      case EDIT_TIME:
        if (editTimeField == 0) {  // Adjust hours
          hourVal--;
          if (hourVal < 0) hourVal = 23;
        } else {                   // Adjust minutes
          minuteVal--;
          if (minuteVal < 0) minuteVal = 59;
        }
        drawEditTime();
        break;
      case EDIT_WEEKDAY:
        weekdayVal--;
        if (weekdayVal < 0) weekdayVal = 6;
        drawEditWeekday();
        break;
      default:
        break;
    }
    encoderAccumulator += currentThreshold;
  }
}

// ----------------------
// Rotary Encoder Callbacks
// ----------------------
void onKnobLeftEventCallback(int count, void *usr_data) {
  static unsigned long lastLeftEventTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastLeftEventTime < encoderDebounceDelay) return;
  lastLeftEventTime = currentTime;

  // Update and print the raw encoder value
  encoderRawValue--;
  Serial.print("EC11 Encoder moved Left, raw value: ");
  Serial.println(encoderRawValue);

  processEncoderInput(-1);
}

void onKnobRightEventCallback(int count, void *usr_data) {
  static unsigned long lastRightEventTime = 0;
  unsigned long currentTime = millis();
  if (currentTime - lastRightEventTime < encoderDebounceDelay) return;
  lastRightEventTime = currentTime;
  
  // Update and print the raw encoder value
  encoderRawValue++;
  Serial.print("EC11 Encoder moved Right, raw value: ");
  Serial.println(encoderRawValue);

  processEncoderInput(+1);
}

// ----------------------
// Setup
// ----------------------
void setup() {
  Serial.begin(115200);
  Serial.println("Combined Menu Example");

  // Initialize TFT display (240x320). Rotate 180° by choosing the proper rotation.
  // For ST7789, rotation values are 0, 1, 2, or 3 where 0 and 2 are 180° apart.
  // Here we choose 180° rotation compared to our previous orientation.
  tft.init(240, 320, SPI_MODE2);
  tft.setRotation(1); // Change from 3 to 1 rotates display 180° relative to previous setting.
  tft.fillScreen(ST77XX_BLACK);

  // Setup buttons as inputs with internal pull-ups.
  pinMode(ENTER_BUTTON_PIN, INPUT_PULLUP);
  pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);

  // Initialize the rotary encoder.
  knob = new ESP_Knob(GPIO_NUM_KNOB_PIN_A, GPIO_NUM_KNOB_PIN_B);
  knob->begin();
  knob->attachLeftEventCallback(onKnobLeftEventCallback);
  knob->attachRightEventCallback(onKnobRightEventCallback);

  // Draw the main menu initially.
  drawMainMenu();
}

// ----------------------
// Main loop
// ----------------------
void loop() {
  // --- ENTER Button Handling (encoder's built-in button) ---
  int enterButtonState = digitalRead(ENTER_BUTTON_PIN);
  if (enterButtonState == LOW && lastEnterButtonState == HIGH &&
      (millis() - lastEnterButtonPressTime) > buttonDebounceDelay) {
    lastEnterButtonPressTime = millis();
    encoderAccumulator = 0; // Clear accumulator on press.
    handleEnterButtonPress();
  }
  lastEnterButtonState = enterButtonState;

  // --- BACK Button Handling (K0 button on pin 5) ---
  int backButtonState = digitalRead(BACK_BUTTON_PIN);
  if (backButtonState == LOW && lastBackButtonState == HIGH &&
      (millis() - lastBackButtonPressTime) > buttonDebounceDelay) {
    lastBackButtonPressTime = millis();
    encoderAccumulator = 0; // Clear accumulator on press.
    handleBackButtonPress();
  }
  lastBackButtonState = backButtonState;

  // --- CLOCK SIMULATION UPDATE ---
  if (currentState == CLOCK_RUNNING) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousClockMillis >= 1000) {
      previousClockMillis = currentMillis;
      secondVal++;
      if (secondVal >= 60) {
        secondVal = 0;
        minuteVal++;
        if (minuteVal >= 60) {
          minuteVal = 0;
          hourVal++;
          if (hourVal >= 24) {
            hourVal = 0;
          }
        }
      }
      drawClock();
    }
  }
  
  delay(10); // Loop delay for stability.
}

// ----------------------
// Button Handling Functions
// ----------------------

// The Enter button (encoder button) confirms the current selection.
void handleEnterButtonPress() {
  switch (currentState) {
    case MAIN_MENU:
      if (menuIndex == 0) {
        currentState = EDIT_TIME;
        editTimeField = 0; // Start with editing hours.
        drawEditTime();
      } else if (menuIndex == 1) {
        currentState = EDIT_WEEKDAY;
        drawEditWeekday();
      } else if (menuIndex == 2) {
        currentState = CLOCK_RUNNING;
        previousClockMillis = millis();
        secondVal = 0; // Optionally reset seconds.
        drawClock();
      }
      break;

    case EDIT_TIME:
      if (editTimeField == 0) {
        // Move from editing hours to minutes.
        editTimeField = 1;
        drawEditTime();
      } else {
        // Finish editing time and return to main menu.
        currentState = MAIN_MENU;
        drawMainMenu();
      }
      break;

    case EDIT_WEEKDAY:
      // Confirm weekday selection.
      currentState = MAIN_MENU;
      drawMainMenu();
      break;
      
    case CLOCK_RUNNING:
      // Stop the clock simulation.
      currentState = MAIN_MENU;
      drawMainMenu();
      break;
  }
}

// The Back button (K0) always returns to the main menu.
void handleBackButtonPress() {
  if (currentState != MAIN_MENU) {
    currentState = MAIN_MENU;
    drawMainMenu();
  }
}

// ----------------------
// Display Drawing Functions
// ----------------------
void drawMainMenu() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Main Menu");
  tft.println("");

  // Option 0: Set Time
  if (menuIndex == 0) {
    tft.setTextColor(ST77XX_YELLOW);
    tft.println("> Set Time");
  } else {
    tft.setTextColor(ST77XX_WHITE);
    tft.println("  Set Time");
  }
  
  // Option 1: Set Weekday
  if (menuIndex == 1) {
    tft.setTextColor(ST77XX_YELLOW);
    tft.println("> Set Weekday");
  } else {
    tft.setTextColor(ST77XX_WHITE);
    tft.println("  Set Weekday");
  }
  
  // Option 2: Start Clock
  if (menuIndex == 2) {
    tft.setTextColor(ST77XX_YELLOW);
    tft.println("> Start Clock");
  } else {
    tft.setTextColor(ST77XX_WHITE);
    tft.println("  Start Clock");
  }
}

void drawEditTime() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Set Time");
  tft.println("");

  // Highlight the value currently being edited.
  if (editTimeField == 0) {
    tft.setTextColor(ST77XX_YELLOW);
  } else {
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.print("Hours: ");
  if (hourVal < 10) tft.print("0");
  tft.println(hourVal);

  if (editTimeField == 1) {
    tft.setTextColor(ST77XX_YELLOW);
  } else {
    tft.setTextColor(ST77XX_WHITE);
  }
  tft.print("Minutes: ");
  if (minuteVal < 10) tft.print("0");
  tft.println(minuteVal);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.println();
  tft.println("Press Enter to confirm/edit");
  tft.println("Press Back to cancel");
}

void drawEditWeekday() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.println("Set Weekday");
  tft.println("");

  tft.setTextColor(ST77XX_YELLOW);
  tft.print("Start on: ");
  tft.println(weekDays[weekdayVal]);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.println();
  tft.println("Press Enter to confirm");
  tft.println("Press Back to cancel");
}

void drawClock() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(10, 50);
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_WHITE);
  
  char timeStr[9];
  sprintf(timeStr, "%02d:%02d:%02d", hourVal, minuteVal, secondVal);
  tft.println(timeStr);
  
  tft.setCursor(10, 120);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(weekDays[weekdayVal]);
  
  tft.setCursor(10, 200);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_CYAN);
  tft.println("Press Enter to stop clock");
  tft.println("Press Back to return");
}
