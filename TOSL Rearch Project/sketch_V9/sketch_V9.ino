#include <Wire.h>
#include <PCF8574.h>
#include <AccelStepper.h>
#include <SerialCommand.h>
#include <LiquidCrystal_I2C.h>

// --- Filter positions for System A ---
#define AF1 28
#define AF2 100
#define AF3 172
#define AF4 244

// --- Filter positions for System B ---
#define BF1 39
#define BF2 111
#define BF3 183
#define BF4 255

// --- System A pins ---
#define sensorPinA 3         // Home sensor (interrupt pin)
#define buttonFilterA P4     // CW button
#define buttonFilterACCW P5  // CCW button
#define buttonHomeA P6       // Home button

// --- System B pins ---
#define sensorPinB 2         // Home sensor (interrupt pin)
#define buttonFilterB P4     // CW button (on second PCF)
#define buttonFilterBCCW P5  // CCW button (on second PCF)
#define buttonHomeB P6       // Home button (on second PCF)

// --- I²C expanders (different addresses) ---
PCF8574 pcfA(0x38);  // System A
PCF8574 pcfB(0x3F);  // System B

// --- Motor parameters ---
int valueS_A = 15000;
int valueA_A = 7000;
int valueS_B = 12000;
int valueA_B = 5000;
int speed = 350;
int dt = 20;

// --- State variables for both systems ---
int pressCountA = 0, pressCountB = 0;
bool lastFilterButtonA = LOW, lastFilterButtonACCW = LOW, lastHomeButtonA = LOW;
bool lastFilterButtonB = LOW, lastFilterButtonBCCW = LOW, lastHomeButtonB = LOW;
volatile bool homeFoundA = false, homeFoundB = false;

// --- Mechanical config ---
const int stepsPerRevolution = 96;
const int totalGearSteps = 288;

// --- LCD Display (shared, 16x2) ---
LiquidCrystal_I2C lcd(0x27, 16, 2);
SerialCommand sCmd;

// --- Custom stepper classes for each PCF8574 ---
class PCFStepperA : public AccelStepper {
public:
  PCFStepperA() : AccelStepper(AccelStepper::HALF4WIRE, 0, 0, 0, 0, false) {}
protected:
  void setOutputPins(uint8_t mask) override {
    pcfA.digitalWrite(0, (mask & 0b0001) ? HIGH : LOW);
    pcfA.digitalWrite(1, (mask & 0b0010) ? HIGH : LOW);
    pcfA.digitalWrite(2, (mask & 0b0100) ? HIGH : LOW);
    pcfA.digitalWrite(3, (mask & 0b1000) ? HIGH : LOW);
  }
};

class PCFStepperB : public AccelStepper {
public:
  PCFStepperB() : AccelStepper(AccelStepper::HALF4WIRE, 0, 0, 0, 0, false) {}
protected:
  void setOutputPins(uint8_t mask) override {
    pcfB.digitalWrite(0, (mask & 0b0001) ? HIGH : LOW);
    pcfB.digitalWrite(1, (mask & 0b0010) ? HIGH : LOW);
    pcfB.digitalWrite(2, (mask & 0b0100) ? HIGH : LOW);
    pcfB.digitalWrite(3, (mask & 0b1000) ? HIGH : LOW);
  }
};

PCFStepperA stepperA;
PCFStepperB stepperB;

void setup() {
  Wire.begin();
  Wire.setClock(400000);
  pcfA.begin();
  pcfB.begin();
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  
  // Configure home sensors 
  pinMode(sensorPinA, INPUT_PULLUP);
  pinMode(sensorPinB, INPUT_PULLUP);
  
  // Initialize PCF A
  for (int i = 0; i < 4; i++) {
    pcfA.pinMode(i, OUTPUT);
    pcfA.digitalWrite(i, LOW);
  }
  pcfA.pinMode(buttonFilterA, INPUT);
  pcfA.pinMode(buttonFilterACCW, INPUT);
  pcfA.pinMode(buttonHomeA, INPUT);
  
  // Initialize PCF B
  for (int i = 0; i < 4; i++) {
    pcfB.pinMode(i, OUTPUT);
    pcfB.digitalWrite(i, LOW);
  }
  pcfB.pinMode(buttonFilterB, INPUT);
  pcfB.pinMode(buttonFilterBCCW, INPUT);
  pcfB.pinMode(buttonHomeB, INPUT);
  
  lastHomeButtonA = pcfA.digitalRead(buttonHomeA);
  lastHomeButtonB = pcfB.digitalRead(buttonHomeB);
  
  Serial.println("Commands: AF1-4, BF1-4, AHome, BHome, ASpeed, BSpeed, AAccel, BAccel, WhereA, WhereB");
  
  // Serial commands
  sCmd.addCommand("AF1", af1);
  sCmd.addCommand("AF2", af2);
  sCmd.addCommand("AF3", af3);
  sCmd.addCommand("AF4", af4);
  sCmd.addCommand("BF1", bf1);
  sCmd.addCommand("BF2", bf2);
  sCmd.addCommand("BF3", bf3);
  sCmd.addCommand("BF4", bf4);
  sCmd.addCommand("AHome", goHomeA);
  sCmd.addCommand("BHome", goHomeB);
  sCmd.addCommand("ASpeed", cmdSetSpeedA);
  sCmd.addCommand("BSpeed", cmdSetSpeedB);
  sCmd.addCommand("AAccel", cmdSetAccelA);
  sCmd.addCommand("BAccel", cmdSetAccelB);
  sCmd.addCommand("WhereA", whereA);
  sCmd.addCommand("WhereB", whereB);
  sCmd.setDefaultHandler(unrecognized);
  
  stepperA.setMaxSpeed(valueS_A);
  stepperB.setMaxSpeed(valueS_B);
  
  // Home both systems
  findHomePositionA();
  findHomePositionB();
}

// --- ISRs ---
void homeInterruptA() { homeFoundA = true; }
void homeInterruptB() { homeFoundB = true; }

// --- Homing functions ---
void findHomePositionA() {
  attachInterrupt(digitalPinToInterrupt(sensorPinA), homeInterruptA, RISING);
  stepperA.setSpeed(speed);
  while (!homeFoundA) stepperA.runSpeed();
  stepperA.stop();
  if (homeFoundA) {
    stepperA.setCurrentPosition(0);
    lcd.setCursor(0,0);
    lcd.print("A:Home  ");
    Serial.println("System A: Home");
  }
  homeFoundA = false;
}

void findHomePositionB() {
  attachInterrupt(digitalPinToInterrupt(sensorPinB), homeInterruptB, RISING);
  stepperB.setSpeed(speed);
  while (!homeFoundB) stepperB.runSpeed();
  stepperB.stop();
  if (homeFoundB) {
    stepperB.setCurrentPosition(0);
    lcd.setCursor(0,1);
    lcd.print("B:Home  ");
    Serial.println("System B: Home");
  }
  homeFoundB = false;
}

// --- Movement functions ---
void moveToFilterA(int targetStep, const char* label) {
  stepperA.setMaxSpeed(valueS_A);
  stepperA.setAcceleration(valueA_A);
  
  long currentPos = stepperA.currentPosition();
  long diff = targetStep - (currentPos % totalGearSteps);
  
  if(diff > totalGearSteps/2) diff -= totalGearSteps;
  if(diff < -totalGearSteps/2) diff += totalGearSteps;
  if(diff == 216) diff = -72;
  
  stepperA.move(diff);
  stepperA.runToPosition();
  
  lcd.setCursor(0, 0);
  lcd.print(label);
  lcd.print("        ");
  Serial.println(label);
}

void moveToFilterB(int targetStep, const char* label) {
  stepperB.setMaxSpeed(valueS_B);
  stepperB.setAcceleration(valueA_B);
  
  long currentPos = stepperB.currentPosition();
  long diff = targetStep - (currentPos % totalGearSteps);
  
  if(diff > totalGearSteps/2) diff -= totalGearSteps;
  if(diff < -totalGearSteps/2) diff += totalGearSteps;
  if(diff == 216) diff = -72;
  
  stepperB.move(diff);
  stepperB.runToPosition();
  
  lcd.setCursor(0, 1);
  lcd.print(label);
  lcd.print("        ");
  Serial.println(label);
}

// --- Filter commands System A ---
void af1() { moveToFilterA(AF1, "A:F1"); }
void af2() { moveToFilterA(AF2, "A:F2"); }
void af3() { moveToFilterA(AF3, "A:F3"); }
void af4() { moveToFilterA(AF4, "A:F4"); }

// --- Filter commands System B ---
void bf1() { moveToFilterB(BF1, "B:F1"); }
void bf2() { moveToFilterB(BF2, "B:F2"); }
void bf3() { moveToFilterB(BF3, "B:F3"); }
void bf4() { moveToFilterB(BF4, "B:F4"); }

void goHomeA() { findHomePositionA(); }
void goHomeB() { findHomePositionB(); }

// --- Where commands ---
void whereA() {
  long pos = stepperA.currentPosition() % totalGearSteps;
  lcd.setCursor(0,0);
  if(pos == AF1) { lcd.print("A:F1   "); Serial.println("A:F1"); }
  else if(pos == AF2) { lcd.print("A:F2   "); Serial.println("A:F2"); }
  else if(pos == AF3) { lcd.print("A:F3   "); Serial.println("A:F3"); }
  else if(pos == AF4 || pos == -33) { lcd.print("A:F4   "); Serial.println("A:F4"); }
  else { lcd.print("A:Home "); Serial.println("A:Home"); }
}

void whereB() {
  long pos = stepperB.currentPosition() % totalGearSteps;
  lcd.setCursor(0,1);
  if(pos == BF1) { lcd.print("B:F1   "); Serial.println("B:F1"); }
  else if(pos == BF2) { lcd.print("B:F2   "); Serial.println("B:F2"); }
  else if(pos == BF3) { lcd.print("B:F3   "); Serial.println("B:F3"); }
  else if(pos == BF4 || pos == -33) { lcd.print("B:F4   "); Serial.println("B:F4"); }
  else { lcd.print("B:Home "); Serial.println("B:Home"); }
}

// --- Speed and Acceleration commands ---
void cmdSetSpeedA() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueS_A = atoi(arg);
    stepperA.setMaxSpeed(valueS_A);
    Serial.print("A Speed: "); Serial.println(valueS_A);
  } else Serial.println("No parameter!");
}

void cmdSetSpeedB() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueS_B = atoi(arg);
    stepperB.setMaxSpeed(valueS_B);
    Serial.print("B Speed: "); Serial.println(valueS_B);
  } else Serial.println("No parameter!");
}

void cmdSetAccelA() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueA_A = atoi(arg);
    stepperA.setAcceleration(valueA_A);
    Serial.print("A Accel: "); Serial.println(valueA_A);
  } else Serial.println("No parameter!");
}

void cmdSetAccelB() {
  char *arg = sCmd.next();
  if(arg != NULL){
    valueA_B = atoi(arg);
    stepperB.setAcceleration(valueA_B);
    Serial.print("B Accel: "); Serial.println(valueA_B);
  } else Serial.println("No parameter!");
}

void unrecognized(const char *command) {
  Serial.print("Unknown: "); Serial.println(command);
}

void goToFilterCircularA(int filterNumber) {
  int targetStep = 0;
  const char* label = "";
  
  if(filterNumber == 1) { targetStep = AF1; label = "A:F1"; }
  else if(filterNumber == 2) { targetStep = AF2; label = "A:F2"; }
  else if(filterNumber == 3) { targetStep = AF3; label = "A:F3"; }
  else if(filterNumber == 4) { targetStep = AF4; label = "A:F4"; }
  
  moveToFilterA(targetStep, label);
}

void goToFilterCircularB(int filterNumber) {
  int targetStep = 0;
  const char* label = "";
  
  if(filterNumber == 1) { targetStep = BF1; label = "B:F1"; }
  else if(filterNumber == 2) { targetStep = BF2; label = "B:F2"; }
  else if(filterNumber == 3) { targetStep = BF3; label = "B:F3"; }
  else if(filterNumber == 4) { targetStep = BF4; label = "B:F4"; }
  
  moveToFilterB(targetStep, label);
}

void loop() {
  sCmd.readSerial();

  // System A buttons
  bool btnA = (pcfA.digitalRead(buttonFilterA)==HIGH);
  bool btnACCW = (pcfA.digitalRead(buttonFilterACCW)==HIGH);
  bool btnHomeA = (pcfA.digitalRead(buttonHomeA)==HIGH);

  if(btnA && !lastFilterButtonA) { 
    pressCountA++; 
    if(pressCountA>4) pressCountA=1; 
    goToFilterCircularA(pressCountA); 
  }
  if(btnACCW && !lastFilterButtonACCW) { 
    pressCountA--; 
    if(pressCountA<1) pressCountA=4; 
    goToFilterCircularA(pressCountA); 
  }
  if(btnHomeA && !lastHomeButtonA) findHomePositionA();

  lastFilterButtonA = btnA;
  lastFilterButtonACCW = btnACCW;
  lastHomeButtonA = btnHomeA;

  // System B buttons
  bool btnB = (pcfB.digitalRead(buttonFilterB)==HIGH);
  bool btnBCCW = (pcfB.digitalRead(buttonFilterBCCW)==HIGH);
  bool btnHomeB_ = (pcfB.digitalRead(buttonHomeB)==HIGH);

  if(btnB && !lastFilterButtonB) { 
    pressCountB++; 
    if(pressCountB>4) pressCountB=1; 
    goToFilterCircularB(pressCountB); 
  }
  if(btnBCCW && !lastFilterButtonBCCW) { 
    pressCountB--; 
    if(pressCountB<1) pressCountB=4; 
    goToFilterCircularB(pressCountB); 
  }
  if(btnHomeB_ && !lastHomeButtonB) findHomePositionB();

  lastFilterButtonB = btnB;
  lastFilterButtonBCCW = btnBCCW;
  lastHomeButtonB = btnHomeB_;

  delay(dt);
}