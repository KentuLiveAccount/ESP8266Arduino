#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ESP32C3 with PCA9685 servo driver breakout
/*
  Wiring

  ESP32C3   PCA9685
  GND    -> GND
  3V3    -> VCC
  GPIO5  -> SDA
  GPIO4  -> SCL
  3V3    -> OE (via switch: ON -> NO POWER to Device, leave open to power device)

  External power (5V) to V+ and GND to power devices

  Note: observing occasional erratic servo movement. supplying separate power to sservo does not help
*/

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40, Wire);

const int sdaPin = 5;
const int sclPin = 4;
void setup() {
  Wire.setPins(sdaPin, sclPin);
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");

  pwm1.begin();
  pwm1.setPWMFreq(50);  // SG90 operates at 50Hz
}

void loop()
{
  // servo sweep from 0 to 180 maps to 120 - 660
  for (int i = 120; i < 661; i += 5)
  {
    Serial.printf("pwm 0: 0, %d\n", i);
    pwm1.setPWM(0, 0, i);
    delay(300);
  }
}