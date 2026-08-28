#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ==========================================
// 1. PIN DEFINITIONS & HARDWARE SETUP
// ==========================================
#define SENSOR_PIN A0    // Analog input from ZMCT103C
#define RELAY_PIN 8      // Digital output to Relay IN
#define GREEN_LED 7      // Digital output for Normal Status LED
#define RED_LED 6        // Digital output for Fault/Trip Status LED
#define RESET_BTN 2      // Digital input for Reset Button

LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// 2. PROTECTION SYSTEM PARAMETERS
// ==========================================
const float ALPHA = 1.0;
const float BETA = 13.5;

// Relay Settings optimized for 12V AC / 1A Transformer Prototype
const float Is = 0.4;       // Pick-up current (A)
const float TMS = 0.05;     // Time Multiplier Setting
const float CALIBRATION_FACTOR = 5.0; 

// ==========================================
// 3. SYSTEM STATE VARIABLES
// ==========================================
bool isTripped = false;           
bool faultActive = false;         
unsigned long faultStartTime = 0; 

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("OCR System Ready");
  lcd.setCursor(0, 1);
  lcd.print("12V AC Prototype");
  delay(2000);
  lcd.clear();
}

void loop() {
  if (isTripped && digitalRead(RESET_BTN) == LOW) {
    resetSystem();
    delay(200);
  }

  if (isTripped) return; 

  float current = readACCurrent();

  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.print(current, 2);
  lcd.print(" A  ");

  if (current > Is) {
    if (!faultActive) {
      faultActive = true;
      faultStartTime = millis(); 
    }

    float tripTimeSeconds = (BETA * TMS) / (pow((current / Is), ALPHA) - 1.0);
    unsigned long tripTimeMillis = tripTimeSeconds * 1000;

    lcd.setCursor(0, 1);
    lcd.print("Fault! t:");
    lcd.print(tripTimeSeconds, 2);
    lcd.print("s  ");

    if ((millis() - faultStartTime) >= tripTimeMillis) {
      triggerRelay();
    }
  } 
  else {
    faultActive = false;
    lcd.setCursor(0, 1);
    lcd.print("Status: NORMAL  ");
  }
  delay(50);
}

// ==========================================
// 4. CUSTOM FUNCTIONS
// ==========================================
float readACCurrent() {
  uint32_t samplePeriod = 40; 
  uint32_t t_start = millis();
  int max_val = 0;
  int min_val = 1023; 
  
  while (millis() - t_start < samplePeriod) {
    int adc_val = analogRead(SENSOR_PIN);
    if (adc_val > max_val) max_val = adc_val;
    if (adc_val < min_val) min_val = adc_val;
  }
  
  float v_swing = ((max_val - min_val) / 1023.0) * 5.0; 
  float v_rms = v_swing / 2.8284; 
  float calculated_current = v_rms * CALIBRATION_FACTOR; 
  if(calculated_current < 0.08) calculated_current = 0.0;
  
  return calculated_current;
}

void triggerRelay() {
  isTripped = true;
  digitalWrite(RELAY_PIN, LOW);    
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, HIGH);  

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM TRIPPED!");
  lcd.setCursor(0, 1);
  lcd.print("Press to Reset");
}

void resetSystem() {
  isTripped = false;
  faultActive = false;
  digitalWrite(RELAY_PIN, HIGH);   
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, HIGH);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Reset...");
  delay(1000);
  lcd.clear();
}
