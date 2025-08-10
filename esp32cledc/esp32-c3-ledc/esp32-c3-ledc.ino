
#if 0
const int ledPin = 0; //4 works too!
 
void setup() {
  // put your setup code here, to run once:
  ledcSetup(0 /*channel*/, 200 /*PWM frequency*/, 8 /*resolution*/);
  ledcAttachPin(ledPin, 0 /*channel*/);
}

void loop() {
  // put your main code here, to run repeatedly:
  static uint8_t brightness = 0;
  static int diff = 5;
 
  ledcWrite(0, brightness);
 
  if (brightness == 25) {
    diff = 1;
  } else if (brightness == 130) {
    diff = -1;
  }
 
  brightness += diff;
  delay(50);
}

// use 12 bit precision for LEDC timer
#define LEDC_TIMER_12_BIT 8

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ 800

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN 0

// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t pin, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(pin, duty);
}

void setup() {
  Serial.begin(9600);
  while(!Serial);
    delay(1000);

  // Setup timer and attach timer to a led pin
  ledcAttach(LED_PIN, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);

  #if 0
  delay(1000);
  ledcAnalogWrite(LED_PIN, 25);
  delay(1000);
  ledcAnalogWrite(LED_PIN, 25);
  delay(1000);
  ledcAnalogWrite(LED_PIN, 180);
  delay(1000);
  ledcAnalogWrite(LED_PIN, 25);
  #endif //0
}

int brightness = 0;  // how bright the LED is
int fadeAmount = 1;  // how many points to fade the LED by

void loop() {
  // set the brightness on LEDC channel 0
  Serial.printf(" Brightness: %d\n", brightness);

  ledcAnalogWrite(LED_PIN, brightness);

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;

  if (brightness > 180)
    brightness = 0;
  #if 0
  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness > 180) {
    fadeAmount = -fadeAmount;
  }
  #endif //0
  // wait for 30 milliseconds to see the dimming effect
  delay(1000);
}

#endif //0

#include <pwmWrite.h>

Pwm pwm = Pwm();

const int servoPin4 = 0;
const int servoPin5 = 1;

void setup() {
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(20, OUTPUT);
  pinMode(21, OUTPUT);
}

void loop() {
  int position;  // position in degrees
  for (position = 0; position <= 180; position += 1) {
    pwm.writeServo(0, position);
    pwm.writeServo(1, 180-position);
    pwm.writeServo(2, position);
    pwm.writeServo(3, 180-position);
    pwm.writeServo(4, position);
    pwm.writeServo(5, 180-position);
    pwm.writeServo(6, position);
    pwm.writeServo(7, 180-position);
    pwm.writeServo(8, position);
    pwm.writeServo(9, 180-position);
    pwm.writeServo(10, position);
    pwm.writeServo(20, 180-position);
    pwm.writeServo(21, position);
    delay(200);
  }
}