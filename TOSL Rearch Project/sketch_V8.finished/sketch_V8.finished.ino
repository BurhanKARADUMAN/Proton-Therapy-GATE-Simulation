#include <Wire.h>               // I²C communication library
#include <PCF8574.h>            // Library for the I²C port expander
#include <AccelStepper.h>       // Library for stepper motor control
#include <SerialCommand.h>      // Serial command parser
#include <LiquidCrystal_I2C.h>  // Library for I²C LCD display

// --- Stepper motor step positions for each optical filter slot ---
#define F1 39
#define F2 111
#define F3 183
#define F4 255

// --- Input pins definitions ---
#define sensorPin 3         // Home sensor input (connected to Arduino interrupt pin)
#define buttonFilterPin P4  // Clockwise filter change button (on PCF8574)
#define buttonFilterPin1 P5 // Counter-clockwise filter change button (on PCF8574)
#define buttonHomePin P6    // Home button (on PCF8574)

// --- I²C expander address ---
PCF8574 pcf(0x38);

// --- Stepper motor parameters ---
int valueS = 12000;  // Maximum motor speed in steps/sec
int valueA = 5000;   // Acceleration in steps/sec²
int speed = 350;     // Speed used during homing
int dt = 50;         // Debounce delay (ms)

// --- State variables ---
int pressCount = 0;  // Keeps track of selected filter index
bool lastFilterButton = LOW, lastFilterButton1 = LOW, lastHomeButton = LOW;
volatile bool homeFound = false; // Flag set when the home sensor triggers

// --- Mechanical configuration ---
const int stepsPerRevolution = 96; // Motor steps per one full revolution
const int totalGearSteps = 288;    // Total steps for one full rotation of the filter wheel

// --- Hardware objects ---
LiquidCrystal_I2C lcd(0x27, 16, 2); // I²C LCD module (address 0x27, 16x2)
SerialCommand sCmd;                 // Serial command handler

// --- Custom stepper class using PCF8574 I/O expander ---
class PCFStepper : public AccelStepper {
public:
  PCFStepper() : AccelStepper(AccelStepper::HALF4WIRE, 0, 0, 0, 0, false) {}
  
protected:
  void setOutputPins(uint8_t mask) override {
    // Override to control stepper coil pins via PCF8574 instead of Arduino GPIO
    pcf.digitalWrite(0, (mask & 0b0001) ? HIGH : LOW);
    pcf.digitalWrite(1, (mask & 0b0010) ? HIGH : LOW);
    pcf.digitalWrite(2, (mask & 0b0100) ? HIGH : LOW);
    pcf.digitalWrite(3, (mask & 0b1000) ? HIGH : LOW);
  }
};

// --- Stepper object (driven through PCF8574) ---
PCFStepper stepper;

void setup() {
  Wire.begin();
  pcf.begin();
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  
  // Configure input pin (home sensor) with pull-up resistor
  pinMode(sensorPin, INPUT_PULLUP);
  
  // Initialize PCF8574 pins
  // Pins 0–3 control the stepper motor coils (outputs)
  for (int i = 0; i < 4; i++) {
    pcf.pinMode(i, OUTPUT);
    pcf.digitalWrite(i, LOW); // Default LOW state
  }
  
  // Buttons are configured as inputs
  pcf.pinMode(buttonFilterPin, INPUT);
  pcf.pinMode(buttonFilterPin1, INPUT);
  pcf.pinMode(buttonHomePin, INPUT);
  
  lastHomeButton = pcf.digitalRead(buttonHomePin);
  
  // Display available serial commands
  Serial.println("Commands: F1, F2, F3, F4, Home, SPEED, ACCEL, Where");
  
  // Map serial commands to their corresponding functions
  sCmd.addCommand("F1", selectFirst);
  sCmd.addCommand("F2", selectSecond);
  sCmd.addCommand("F3", selectThird);
  sCmd.addCommand("F4", selectFourth);
  sCmd.addCommand("Home", goHome);
  sCmd.addCommand("SPEED", cmdSetS);
  sCmd.addCommand("ACCEL", cmdSetA);
  sCmd.addCommand("Where", where);
  sCmd.setDefaultHandler(unrecognized);
  
  // Set initial maximum speed
  stepper.setMaxSpeed(valueS);
  
  // Automatically find the home position on startup
  findHomePosition();
}

// --- Displays the current filter position ---
void where() {
  int pos = stepper.currentPosition();
  long Fpos = pos % totalGearSteps;
  
  if (Fpos == F1) { lcd.clear(); lcd.print("On filter 1"); Serial.println("On filter 1"); }
  if (Fpos == F2) { lcd.clear(); lcd.print("On filter 2"); Serial.println("On filter 2"); }
  if (Fpos == F3) { lcd.clear(); lcd.print("On filter 3"); Serial.println("On filter 3"); }
  if (Fpos == F4 || Fpos == -33) { lcd.clear(); lcd.print("On filter 4"); Serial.println("On filter 4"); }
  if (Fpos == 0) { lcd.clear(); lcd.print("On Home"); Serial.println("On Home"); }
}

// --- Interrupt Service Routine: triggered by the home sensor ---
void homeInterrupt() {
  homeFound = true;
}

// --- Function to locate the home position automatically ---
void findHomePosition() {
  attachInterrupt(digitalPinToInterrupt(sensorPin), homeInterrupt, RISING);
  stepper.setSpeed(speed);
  
  while (!homeFound) {
    stepper.runSpeed();
  }
  stepper.stop();
  
  if (homeFound) {
    Serial.println("Home found. Ready for filters.");
    stepper.setCurrentPosition(0);
    lcd.clear();
    lcd.print("On Home");
    lcd.setCursor(0,1);
    lcd.print("READY");
  } else {
    Serial.println("Home not found.");
  }
  homeFound = false;
}

// --- Serial command: set motor speed ---
void cmdSetS() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueS = atoi(arg);
    stepper.setMaxSpeed(valueS);
    Serial.print("Speed set to: "); Serial.println(valueS);
  } else Serial.println("No parameter!");
}

// --- Serial command: set motor acceleration ---
void cmdSetA() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueA = atoi(arg);
    stepper.setAcceleration(valueA);
    Serial.print("Acceleration set to: "); Serial.println(valueA);
  } else Serial.println("No parameter!");
}

// --- Moves the stepper to the target filter position ---
void moveToFilter(int targetStep, const char* label) {
  stepper.setMaxSpeed(valueS);
  stepper.setAcceleration(valueA);
  
  long currentPos = stepper.currentPosition();
  long diff = targetStep - (currentPos % totalGearSteps);
  
  // Handle shortest path rotation
  if(diff > totalGearSteps/2) diff -= totalGearSteps;
  if(diff < -totalGearSteps/2) diff += totalGearSteps;
  if(diff == 216) diff = -72;
  
  stepper.move(diff);
  stepper.runToPosition();
  
  lcd.clear();
  lcd.print("On "); lcd.print(label);
  Serial.print("On "); Serial.println(label);
}

// --- Filter selection shortcuts ---
void selectFirst() { moveToFilter(F1, "F1"); }
void selectSecond() { moveToFilter(F2, "F2"); }
void selectThird() { moveToFilter(F3, "F3"); }
void selectFourth() { moveToFilter(F4, "F4"); }

// --- Returns to home position ---
void goHome() { findHomePosition(); }

// --- Handles unrecognized serial commands ---
void unrecognized(const char *command) {
  Serial.print("Unknown command: "); Serial.println(command);
  Serial.println("Use: F1, F2, F3, F4, Home, SPEED, ACCEL, Where");
}

// --- Moves circularly between filters using button presses ---
void goToFilterCircular(int filterNumber) {
  int targetStep = 0;
  const char* label = "";
  
  if(filterNumber == 1){targetStep = F1; label = "F1";}
  if(filterNumber == 2){targetStep = F2; label = "F2";}
  if(filterNumber == 3){targetStep = F3; label = "F3";}
  if(filterNumber == 4){targetStep = F4; label = "F4";}
  
  moveToFilter(targetStep, label);
}

// --- Main loop: handles buttons and serial commands ---
void loop() {
  sCmd.readSerial(); // Continuously listen for serial commands

  // Read the state of PCF8574 buttons
  bool currentFilterButton = (pcf.digitalRead(buttonFilterPin)==HIGH);
  bool currentFilterButton1 = (pcf.digitalRead(buttonFilterPin1)==HIGH);
  bool currentHomeButton = (pcf.digitalRead(buttonHomePin)==HIGH);

  // Clockwise filter rotation
  if(currentFilterButton==HIGH && lastFilterButton==LOW){ 
    pressCount++; 
    if(pressCount>4) pressCount=1; 
    goToFilterCircular(pressCount); 
  }

  // Counter-clockwise rotation
  if(currentFilterButton1==HIGH && lastFilterButton1==LOW){ 
    pressCount--; 
    if(pressCount<1) pressCount=4; 
    goToFilterCircular(pressCount); 
  }

  // Home button rising edge
  if(currentHomeButton==HIGH && lastHomeButton==LOW){ 
    findHomePosition(); 
  }

  // Update button state memory
  lastHomeButton=currentHomeButton;
  lastFilterButton=currentFilterButton;
  lastFilterButton1=currentFilterButton1;
  
  delay(dt); // Small delay for button debounce
}
