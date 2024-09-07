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

const int elbowRF = 0;
const int elbowRB = 1;
const int elbowLF = 2;
const int elbowLB = 3;
const int shoulderRF = 4;
const int shoulderRB = 5;
const int shoulderLF = 6;
const int shoulderLB = 7;

void setup() {
  Wire.setPins(sdaPin, sclPin);
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");

  pwm1.begin();
  pwm1.setPWMFreq(50);  // SG90 operates at 50Hz
}

void fullSweepForever()
{
  // servo sweep from 0 to 180 maps to 100 - 550 (changes with frequencies)
  for (int i = 100; i < 551; i += 5)
  {
    Serial.printf("pwm 0: 0, %d\n", i);
    pwm1.setPWM(0, 0, i);
    delay(300);
  }

}

void nearZeroTest()
{
    pwm1.setPWM(0, 0, 100); // zero is 100
    delay(1000);
    pwm1.setPWM(0, 0, 550); // 180 is 550
    delay(1000);

  int v = 90;
  for (;;)
  {
    Serial.printf("pwm 0: 0, %d\n", v);
    pwm1.setPWM(0, 0, v);
    delay(300);
    if (v == 90) v = 95;
    else if (v == 95) v = 100;
    else if (v == 100) v = 105;
    else if (v == 105) v = 90;
  }
}

void near180Test()
{
  int v = 545;
  for (;;)
  {
    Serial.printf("pwm 0: 0, %d\n", v);
    pwm1.setPWM(0, 0, v);
    delay(300);
    if (v == 545) v = 550;
    else if (v == 550) v = 555;
    else if (v == 555) v = 560;
    else if (v == 560) v = 545;
  }
}


void sweepAnd90(int servo)
{
    pwm1.setPWM(servo, 0, 100); // zero is 100
    delay(1000);
    pwm1.setPWM(servo, 0, 550); // 180 is 550
    delay(1000);
    pwm1.setPWM(servo, 0, 325); // 90 is 325 (100 + 450 / 2)
    delay(1000);
}

void sweepAndRestAt90()
{
  sweepAnd90 (0);
}

void sweepAndRestAt90Elbows()
{
  sweepAnd90 (elbowRF);
  sweepAnd90 (elbowRB);
  sweepAnd90 (elbowLF);
  sweepAnd90 (elbowLB);
}

void sweeplessAnd90(int servo)
{
    pwm1.setPWM(servo, 0, 212); // zero is 100
    delay(1000);
    pwm1.setPWM(servo, 0, 438); // 180 is 550
    delay(1000);
    pwm1.setPWM(servo, 0, 325); // 90 is 325 (100 + 450 / 2)
    delay(1000);
}

void sweepAndRestAt90Shoulders()
{
  sweeplessAnd90 (shoulderRF);
  sweeplessAnd90 (shoulderRB);
  sweeplessAnd90 (shoulderLF);
  sweeplessAnd90 (shoulderLB);
}

void home()
{
  pwm1.setPWM(elbowRF, 0, 325);  
  pwm1.setPWM(elbowRB, 0, 325);  
  pwm1.setPWM(elbowLF, 0, 325);  
  pwm1.setPWM(elbowLB, 0, 325);  
  pwm1.setPWM(shoulderRF, 0, 325);  
  pwm1.setPWM(shoulderRB, 0, 325);  
  pwm1.setPWM(shoulderLF, 0, 325);  
  pwm1.setPWM(shoulderLB, 0, 325);  
}

void loop()
{
  home();
  delay(1000);
  sweepAndRestAt90Shoulders();

  for (;;)
  {
    delay(300);
  }
}