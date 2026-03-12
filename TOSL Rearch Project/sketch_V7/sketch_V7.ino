#include <Wire.h>
#include <PCF8574.h>
#include <AccelStepper.h>

PCF8574 pcf(0x38);  // PCF8574 I2C adresi (jumper ayarına göre değişebilir)

// PCF8574 çıkış durumunu saklamak için
byte pcfOutputState = 0x00;

// PCF8574’e yazma fonksiyonu
void writePCF8574(byte value) {
  pcfOutputState = value;
  pcf.write8(pcfOutputState);  // RobTillaart kütüphanesindeki fonksiyon
}

// AccelStepper için özelleştirilmiş class
class PCFStepper : public AccelStepper {
  public:
    PCFStepper() : AccelStepper(AccelStepper::FULL4WIRE, 0, 0, 0, 0, false) {}

  protected:
    void setOutputPins(uint8_t mask) override {
      // Motor pinlerini ayarla, diğer pinleri koru
      byte output = (pcfOutputState & 0xF0) | (mask & 0x0F);
      writePCF8574(output);
    }
};

// Stepper nesnesi (PCF üzerinden sürülecek)
PCFStepper stepper;

void setup() {
  Wire.begin();
  pcf.begin();

  // Başlangıç çıkışları sıfırla
  writePCF8574(0x00);

  stepper.setMaxSpeed(300.0);
  stepper.setAcceleration(100.0);
  stepper.moveTo(200);  // örnek hedef adım
}

void loop() {
  stepper.run();
}
