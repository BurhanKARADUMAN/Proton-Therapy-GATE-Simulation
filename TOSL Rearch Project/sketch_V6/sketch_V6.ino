#include <AccelStepper.h>      // Library for controlling stepper motors
#include <SerialCommand.h>     // Library for parsing serial commands
#include <LiquidCrystal_I2C.h> // Library for controlling I2C LCD displays
#include <Wire.h>

// --- Motor pin connections ---
#define motorPin1 8
#define motorPin2 9
#define motorPin3 10
#define motorPin4 11

// --- Stepper motor positions for each filter ---
#define F1 39
#define F2 111
#define F3 183
#define F4 255

// --- Input pins ---
int sensorPin = 3;          // Home sensor (interrupt)
int buttonFilterPin = 6;    // CW filter change button
int buttonFilterPin1 = 5;   // CCW filter change button
int buttonHomePin = 2;      // Home button

// --- Motor parameters ---
int valueS = 10000;         // Maximum speed (steps/sec)
int valueA = 5000;          // Acceleration (steps/sec²)
int speed  = 400;           // Speed for homing

// --- State variables ---
int pressCount = 0;         
bool lastFilterButton = HIGH, lastFilterButton1 = HIGH, lastHomeButton = LOW;
volatile bool homeFound = false; // Flag when home detected

// --- Mechanical configuration ---
const int stepsPerRevolution = 96;   // Motor steps per full revolution
const int totalGearSteps = 288;      // Steps for full filter wheel rotation

// --- Objects ---
AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin2, motorPin3, motorPin4);
LiquidCrystal_I2C lcd(0x27, 16, 2);  // LCD object (I2C address 0x27)
SerialCommand sCmd;                  // Serial command parser

// -------------------- SETUP --------------------
void setup() {
  Serial.begin(9600);
  lcd.begin();

  // Configure pins (buttons with internal pull-ups)
  pinMode(sensorPin, INPUT);
  pinMode(buttonFilterPin, INPUT_PULLUP);
  pinMode(buttonFilterPin1, INPUT_PULLUP);
  pinMode(buttonHomePin, INPUT_PULLUP);

  lastHomeButton = digitalRead(buttonHomePin);

  // Show available commands
  Serial.println("Commands: F1, F2, F3, F4, Home");

  // Map serial commands to functions
  sCmd.addCommand("F1", selectFirst);
  sCmd.addCommand("F2", selectSecond);
  sCmd.addCommand("F3", selectThird);
  sCmd.addCommand("F4", selectFourth);
  sCmd.addCommand("Home", goHome);
  sCmd.addCommand("SPEED", cmdSetS);
  sCmd.addCommand("ACCEL", cmdSetA);
  sCmd.addCommand("Where", where);
  sCmd.setDefaultHandler(unrecognized);

  stepper.setMaxSpeed(valueS);

  // Automatically find home position on startup
  findHomePosition();
}

// -------------------- POSITION CHECK --------------------
void where() { // Show current filter on serial and LCD
  int pos = stepper.currentPosition();
  long Fpos = pos % totalGearSteps;
  if (Fpos == F1) { Serial.println("On filter 1"); lcd.clear(); lcd.setCursor(0,0); lcd.print("On filter 1"); }
  if (Fpos == F2) { Serial.println("On filter 2"); lcd.clear(); lcd.setCursor(0,0); lcd.print("On filter 2"); }
  if (Fpos == F3) { Serial.println("On filter 3"); lcd.clear(); lcd.setCursor(0,0); lcd.print("On filter 3"); }
  if (Fpos == F4 || Fpos == -33) { Serial.println("On filter 4"); lcd.clear(); lcd.setCursor(0,0); lcd.print("On filter 4"); }
  if (Fpos == 0) { Serial.println("On Home"); lcd.clear(); lcd.setCursor(0,0); lcd.print("On Home"); }
}

// -------------------- INTERRUPT --------------------
void homeInterrupt() { homeFound = true; } // Set flag when home sensor triggers

// -------------------- HOME POSITION --------------------
void findHomePosition() {
  homeFound = false;
  attachInterrupt(digitalPinToInterrupt(sensorPin), homeInterrupt, RISING); // Trigger on rising edge
  stepper.setSpeed(speed); // Set constant speed for homing

  unsigned long startTime = millis();
  while (!homeFound && millis() - startTime < 20000) { stepper.runSpeed(); } // Run until home found or timeout
  stepper.stop();

  if (homeFound) { // Reset position and show LCD
    Serial.println("Home found. Ready for filters.");
    stepper.setCurrentPosition(0);
    lcd.clear(); lcd.setCursor(0,0); lcd.print("On Home"); lcd.setCursor(0,1); lcd.print("READY");
    delay(100);
    moveToFilter(F1,"F1"); pressCount=1; // Move to first filter
  } else Serial.println("Home not found.");
}

// -------------------- SERIAL COMMANDS --------------------
void cmdSetS() { char *arg = sCmd.next(); if(arg!=NULL){valueS=atoi(arg); stepper.setMaxSpeed(valueS);} else Serial.println("No parameter!"); }
void cmdSetA() { char *arg = sCmd.next(); if(arg!=NULL){valueA=atoi(arg); stepper.setAcceleration(valueA);} else Serial.println("No parameter!"); }

// -------------------- MOVE TO FILTER --------------------
void moveToFilter(int targetStep, const char* label) {
  stepper.setMaxSpeed(valueS); stepper.setAcceleration(valueA);
  long currentPos = stepper.currentPosition();
  long diff = targetStep - (currentPos % totalGearSteps); // Shortest rotation
  if(diff>totalGearSteps/2) diff-=totalGearSteps;
  if(diff<-totalGearSteps/2) diff+=totalGearSteps;
  if(diff==216) diff=-72; // special correction
  stepper.move(diff); stepper.runToPosition(); // Move to target
  lcd.clear(); lcd.setCursor(0,0); lcd.print("On "); lcd.print(label);
  Serial.print("On "); Serial.println(label);
}

// -------------------- FILTER SELECTION --------------------
void selectFirst()  { moveToFilter(F1,"F1"); }
void selectSecond() { moveToFilter(F2,"F2"); }
void selectThird()  { moveToFilter(F3,"F3"); }
void selectFourth() { moveToFilter(F4,"F4"); }

// -------------------- GO HOME --------------------
void goHome() { findHomePosition(); }

// -------------------- UNRECOGNIZED COMMAND --------------------
void unrecognized() { Serial.println("Use: F1, F2, F3, F4, Home"); }

// -------------------- CIRCULAR FILTER CHANGE --------------------
void goToFilterCircular(int filterNumber) {
  int targetStep=0; const char* label="";
  if(filterNumber==1){targetStep=F1; label="F1";}
  if(filterNumber==2){targetStep=F2; label="F2";}
  if(filterNumber==3){targetStep=F3; label="F3";}
  if(filterNumber==4){targetStep=F4; label="F4";}
  moveToFilter(targetStep,label); // Reuse existing move function
}

// -------------------- MAIN LOOP --------------------
void loop() {
  sCmd.readSerial(); // Check serial commands

  // Read button states
  bool currentFilterButton = digitalRead(buttonFilterPin);
  bool currentFilterButton1 = digitalRead(buttonFilterPin1);
  bool currentHomeButton = digitalRead(buttonHomePin);

  // Clockwise filter change
  if(currentFilterButton==LOW && lastFilterButton==HIGH){ pressCount++; if(pressCount>4) pressCount=1; goToFilterCircular(pressCount); }

  // Counter-clockwise filter change 
  if(currentFilterButton1==LOW && lastFilterButton1==HIGH){ pressCount--; if(pressCount<1) pressCount=4; goToFilterCircular(pressCount); }

  // Home button rising edge detection
  if(currentHomeButton==HIGH && lastHomeButton==LOW){ findHomePosition(); }

  // Update last states
  lastHomeButton=currentHomeButton;
  lastFilterButton=currentFilterButton;
  lastFilterButton1=currentFilterButton1;
}
