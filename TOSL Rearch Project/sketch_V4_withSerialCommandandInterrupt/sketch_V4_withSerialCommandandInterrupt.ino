#include <AccelStepper.h>
#include <SerialCommand.h>  // Librarleri tanımlama
#include <LiquidCrystal_I2C.h>

#define motorPin1 8  
#define motorPin2 9
#define motorPin3 10
#define motorPin4 11     // Motor pinlerini tanımlama

#define F1 24
#define F2 96
#define F3 168
#define F4 240          // Filtrelerin Home Positiona göre konumları

int sensorPin = 3;
int buttonFilterPin =5;
int buttonHomePin =6;   //Kullandığım sensör ve butonların Input pinleri

int maxS=10000;  
int accel=5000; 
int speed = 400;   //Motorun hız ve ivme değişkenleri

int pressCount=0;    //Filtre butonuna her basıldığında gidilecek filtreyi sayan değişken
bool lastFilterButton = LOW;
bool lastHomeButton = LOW;    //Filtre butununda birden çok basılma olmaması için koyduğum koşulların değişkenleri  
volatile bool homeFound = false; //Interrupt için belirlenen değişken

const int stepsPerRevolution = 96;
const int totalGearSteps = 288;     //Motorun ve filtre tekerinin adım değeri

AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin2, motorPin3, motorPin4);  //Motorun tanıtılması

LiquidCrystal_I2C lcd(0x27,16,2);    //LCD ekranın tanıtılması
SerialCommand sCmd; // 1 tane serial command objesi tanımlama
void homeInterrupt() {          // findHomePosition fonksiyonu için her kullanıldığında bool değişkenini döndürecek fonksiyon
  homeFound = true;
}

void findHomePosition() {           // home positionı bulmak için yazılan fonksiyon. Önce tekrar çağırılabilmesi için başta bool değişkeni sıfırlanıyor sonra 1 tane argüman eklenerek(RISING) sensörden gelen bildirime göre motoru döndürüyor ta ki argüman sağlanana kadar.
  homeFound = false;
  attachInterrupt(digitalPinToInterrupt(sensorPin), homeInterrupt, RISING);
  stepper.setSpeed(speed);

  unsigned long startTime = millis();
  int stepsTaken = 0;

  while (!homeFound && millis() - startTime < 200000 && stepsTaken < 6000) {    // sigorta kodu gibi bir şey bu eğer herhangi bir sorundan dolayı home position bulunamazsa geri bildirim verip home positionun bulunamadığı gösterilsin
    stepper.runSpeed();
  }

  stepper.stop();
  //detachInterrupt(digitalPinToInterrupt(sensorPin));    // Interrupt detach edilip filtrelere geçimeye engel olmasın

  if (homeFound) {
    Serial.println("Home position found.");
    stepper.setCurrentPosition(0);
  } else {
    Serial.println("Home not found.");
  }
  pressCount=0; 
}

void moveToFilter(int position, const char* label) {       // Filtre fonksiyonu filtrelerin konumlarına ve Serial Monitörden alınan koşula göre fonksiyon çağırılıp döndürülüyor char* labella bu sağlanıyor çünkü tek parametrede yapılabiliyor
 //findHomePosition();
  //stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(maxS);    
  stepper.setAcceleration(accel); 
  stepper.moveTo(position);
  stepper.runToPosition();   
  Serial.print("Filter selected: ");
  Serial.println(label);
}

void selectFirst()  { moveToFilter(F1, "F1"); }     // burda fonksiyonlar oluşturulup çağırılacakları koşullar belirtiliyor
void selectSecond() { moveToFilter(F2, "F2"); }
void selectThird()  { moveToFilter(F3, "F3"); }
void selectFourth() { moveToFilter(F4, "F4"); }

void goHome() {
  findHomePosition();    // tekrar home a çağırmak için var
}

void unrecognized() {
  Serial.println("Use: F1, F2, F3, F4, Home");   // girilen komut tanımlı değilse verilecek geri bildirim için var
}

void setup() {
  Serial.begin(9600);
  lcd.begin();
  pinMode(sensorPin, INPUT);
  pinMode(buttonFilterPin, INPUT_PULLUP);
  pinMode(buttonHomePin, INPUT_PULLUP);  //Pinlerin input olduğunu tanıtılması
  stepper.setMaxSpeed(maxS);
  findHomePosition();       // ilk çalıştığında 1 kerelik Home Positionu bulsun diye var
  stepper.setCurrentPosition(0);
  Serial.println("Filters: F1, F2, F3, F4");

  sCmd.addCommand("F1", selectFirst);
  sCmd.addCommand("F2", selectSecond);
  sCmd.addCommand("F3", selectThird);
  sCmd.addCommand("F4", selectFourth);
  sCmd.addCommand("Home", goHome);          // command ların eklenmesi bu setup a 1 er kere yapılır ki başlangıçta bulunsun ve aktif olarak memory yemesin arduino da 
  sCmd.setDefaultHandler(unrecognized);
}

void loop() {
 
  sCmd.readSerial();    // her döndüğünde komuta göre doğru konuma gitsin
  bool currentFilterButton = digitalRead(buttonFilterPin);
  bool currentHomeButton   = digitalRead(buttonHomePin);    //Butonların anlık basılma durumunu alıp tekrar tekrar basılmış gibi algılamanın önüne geçmek için var 

  // Filtre butonuna basıldıysa (LOW → HIGH geçişi)
  if (currentFilterButton == HIGH && lastFilterButton == LOW) {
    pressCount++;
    if (pressCount > 4) pressCount = 1;

    if (pressCount == 1) {     //Butonun basılma sayısına göre filtre değiştiriyor
      filter1();
    } else if (pressCount == 2) {
      filter2();
    } else if (pressCount == 3) {
      filter3();
    } else if (pressCount == 4) {
      filter4();
    }
  }

  // Home butonuna basıldıysa Home fonksiyonunu çağırıyor
  if (currentHomeButton == HIGH && lastHomeButton == LOW) {
    findHomePosition();
  }

  // Önceki değerleri güncelliyor ki sürekli basıldığı şeklinde algılanmasın
  lastFilterButton = currentFilterButton;
  lastHomeButton = currentHomeButton;
}
void filter1() {            //Fonksiyon çağırıldığında max hızı ayarlanıp istenen konuma döndürülüyor sonra LED ekran temizlenip anlık bulunduğu yeri yazdırıyor
 stepper.setMaxSpeed(maxS);    
  stepper.setAcceleration(accel); 
  stepper.moveTo(F1);
  stepper.runToPosition();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("On Filter 1"); 
}

void filter2() {             //Fonksiyon çağırıldığında max hızı ayarlanıp istenen konuma döndürülüyor sonra LED ekran temizlenip anlık bulunduğu yeri yazdırıyor
 stepper.setMaxSpeed(maxS);    
  stepper.setAcceleration(accel); 
  stepper.moveTo(F2);
  stepper.runToPosition();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("On Filter 2");
}

void filter3() {                   //Fonksiyon çağırıldığında max hızı ayarlanıp istenen konuma döndürülüyor sonra LED ekran temizlenip anlık bulunduğu yeri yazdırıyor
  stepper.setMaxSpeed(maxS);    
  stepper.setAcceleration(accel); 
  stepper.moveTo(F3);
  stepper.runToPosition();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("On Filter 3");
}

void filter4() {                  //Fonksiyon çağırıldığında max hızı ayarlanıp istenen konuma döndürülüyor sonra LED ekran temizlenip anlık bulunduğu yeri yazdırıyor
 stepper.setMaxSpeed(maxS);    
  stepper.setAcceleration(accel); 
  stepper.moveTo(F4);
  stepper.runToPosition();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("On Filter 4");
}
