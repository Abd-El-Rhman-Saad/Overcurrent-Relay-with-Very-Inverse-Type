#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>

// ==========================================
// 1. PIN DEFINITIONS & HARDWARE SETUP
// ==========================================
#define RELAY_PIN 8      // Pin connected to the 1-Channel Relay Module
#define BUZZER_PIN 9     // Pin connected to the Buzzer
#define RESET_BTN 10     // Pin connected to the Push Button (Reset)

// PZEM-004T Sensor using SoftwareSerial (RX, TX)
#define PZEM_RX 2
#define PZEM_TX 3
SoftwareSerial pzemSWSerial(PZEM_RX, PZEM_TX);
PZEM004Tv30 pzem(pzemSWSerial);

// Initialize LCD (I2C address is usually 0x27 or 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==========================================
// 2. PROTECTION SYSTEM PARAMETERS
// ==========================================
// IEEE Very Inverse Curve Parameters
const float alpha = 1.0;   // Curve constant alpha for Very Inverse
const float beta = 13.5;   // Curve constant beta for Very Inverse

// Relay Settings (Can be adjusted based on requirements)
const float Is = 2.0;      // Current Setting (Pick-up current) in Amperes
const float TMS = 0.05;    // Time Multiplier Setting in Seconds

// ==========================================
// 3. SYSTEM STATE VARIABLES
// ==========================================
bool isTripped = false;           // Tracks if the relay has tripped
bool faultActive = false;         // Tracks if a fault is currently happening
unsigned long faultStartTime = 0; // Records when the fault started

void setup() {
  Serial.begin(9600);
  
  // Configure Pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RESET_BTN, INPUT_PULLUP); // Use internal pull-up resistor

  // Initial State: System Normal (Assuming active LOW relay)
  digitalWrite(RELAY_PIN, HIGH); 
  digitalWrite(BUZZER_PIN, LOW);

  // Initialize LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("OCR System Ready");
  lcd.setCursor(0, 1);
  lcd.print("Very Inverse...");
  delay(2000);
  lcd.clear();
}

void loop() {
  // ---------------------------------------------------------
  // STEP A: Check Reset Button
  // ---------------------------------------------------------
  // If system is tripped and button is pressed, reset it
  if (isTripped && digitalRead(RESET_BTN) == LOW) {
    resetSystem();
  }

  // If the system is currently tripped, halt further current processing
  if (isTripped) {
    return; 
  }

  // ---------------------------------------------------------
  // STEP B: Read Sensor Data
  // ---------------------------------------------------------
  float current = pzem.current();

  // Check if sensor reading is valid
  if (isnan(current)) {
    lcd.setCursor(0, 0);
    lcd.print("Sensor Error!   ");
    return;
  }

  // Display Real-Time Current on LCD
  lcd.setCursor(0, 0);
  lcd.print("Current: ");
  lcd.print(current, 2);
  lcd.print(" A  ");

  // ---------------------------------------------------------
  // STEP C: Fault Detection & Tripping Logic
  // ---------------------------------------------------------
  if (current > Is) {
    // A fault is detected
    if (!faultActive) {
      faultActive = true;
      faultStartTime = millis(); // Record the exact time the fault started
    }

    // Calculate dynamic Trip Time based on the Very Inverse Equation
    // Equation: t = beta * TMS / ((I / Is)^alpha - 1)
    float tripTimeSeconds = (beta * TMS) / (pow((current / Is), alpha) - 1.0);
    unsigned long tripTimeMillis = tripTimeSeconds * 1000;

    // Display fault status and calculated trip time
    lcd.setCursor(0, 1);
    lcd.print("Fault! t:");
    lcd.print(tripTimeSeconds, 2);
    lcd.print("s  ");

    // Check if the fault has persisted longer than the calculated trip time
    if ((millis() - faultStartTime) >= tripTimeMillis) {
      triggerRelay();
    }
  } 
  else {
    // Normal Operating Condition (Current is safe)
    faultActive = false;
    lcd.setCursor(0, 1);
    lcd.print("Status: NORMAL  ");
  }

  delay(100); // Small delay for system stability
}

// ==========================================
// 4. CUSTOM FUNCTIONS
// ==========================================

// Function to activate protection mechanisms
void triggerRelay() {
  isTripped = true;
  
  // Cut off power & sound alarm
  digitalWrite(RELAY_PIN, LOW);    // Activate relay (Disconnect Load)
  digitalWrite(BUZZER_PIN, HIGH);  // Turn on Buzzer

  // Update Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SYSTEM TRIPPED!");
  lcd.setCursor(0, 1);
  lcd.print("Press to Reset");
}

// Function to restore normal operation
void resetSystem() {
  isTripped = false;
  faultActive = false;
  
  // Restore power & silence alarm
  digitalWrite(RELAY_PIN, HIGH);   // Deactivate relay (Reconnect Load)
  digitalWrite(BUZZER_PIN, LOW);   // Turn off Buzzer

  // Update Display
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("System Reset...");
  delay(1000);
  lcd.clear();
}