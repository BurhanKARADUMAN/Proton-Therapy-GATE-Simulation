#include <AccelStepper.h>
#include <SerialCommands.h>

#define motorPin1 8
#define motorPin2 9
#define motorPin3 10
#define motorPin4 11

#define F1 24
#define F2 96
#define F3 -120
#define F4 -28

char buffer[32];
SerialCommands serial(&Serial, buffer, sizeof(buffer), "\r\n", " ");
const int stepsPerRevolution = 48;
const int totalGearSteps = 144;
int speed = 10;
int sensorPin = 2;
//int ledPin = 3;
int dt = 100;
String msg = "Choose your filter(First, Second, Third, Fourth):";
String commend;

AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin2, motorPin3, motorPin4);

volatile bool homeFound = false;


void setup() {

  SerialCommand filtre1("First", filter1);
  SerialCommand filtre2("Second", filter2);
  SerialCommand filtre3("Third", filter3);
  SerialCommand filtre4("Fourth", filter4);
  SerialCommand goHome("Home", findHomePosition);
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(sensorPin), homeInterrupt, RISING);
  stepper.setMaxSpeed(20);
  //stepper.setAcceleration(10);
  findHomePosition();
  stepper.setCurrentPosition(0);
  Serial.println(msg);
  serial.AddCommand(&filtre1);
  serial.AddCommand(&filtre2);
  serial.AddCommand(&filtre3);
  serial.AddCommand(&filtre4);
  serial.AddCommand(&goHome);
}


void homeInterrupt() {
  homeFound = true;
  stepper.stop();
  Serial.println("Home Position have been found.");
  stepper.setCurrentPosition(0);
}

void findHomePosition() {
  homeFound = false;
  stepper.setSpeed(speed);
  unsigned long startTime = millis();
  int stepsTaken = 0;
  while (!homeFound && millis() - startTime < 20000 && stepsTaken < 600) {
    stepper.runSpeed();
    Serial.println(stepper.currentPosition());

    delay(dt);
    stepsTaken++;
  }

  delay(dt);
}



void loop() {
  //Serial.println(msg);
  serial.ReadSerial();
}
void filter1(SerialCommands* sender) {
  //findHomePosition();
  stepper.moveTo(F1);
  sender->GetSerial()->println("Filter 1 selected");
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void filter2(SerialCommands* sender) {
  //findHomePosition();
  stepper.moveTo(F2);
  sender->GetSerial()->println("Filter 2 selected");
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void filter3(SerialCommands* sender) {
  findHomePosition();
  stepper.moveTo(F3);
  sender->GetSerial()->println("Filter 3 selected");
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}

void filter4(SerialCommands* sender) {
  findHomePosition();
  stepper.moveTo(F4);
  sender->GetSerial()->println("Filter 4 selected");
  while (stepper.distanceToGo() != 0) {
    stepper.run();
  }
}
