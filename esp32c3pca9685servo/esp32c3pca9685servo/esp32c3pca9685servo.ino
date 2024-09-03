#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40, Wire);

const int sdaPin = 5;
const int sclPin = 4;
void setup() {
  Wire.setPins(sdaPin, sclPin);
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");

  pwm1.begin();
  pwm1.setPWMFreq(60);  // This is the maximum PWM frequency
}

void loop()
{
  for (int i = 120; i < 661; i += 5)
  {
    Serial.printf("pwm 0: 0, %d\n", i);
    pwm1.setPWM(0, 0, i);
    delay(300);
  }
}