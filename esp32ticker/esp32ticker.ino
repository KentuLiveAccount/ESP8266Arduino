#include <Arduino.h>
#include <Ticker.h>

// attach a LED to pPIO 21
#define LED_PIN 5 //0, 1, 2, 3, 4, 7, 8 (these are GPIO port num)

Ticker blinker;
Ticker toggler;
Ticker changer;
float blinkerPace = 0.1;  //seconds
const float togglePeriod = 5; //seconds

void change() {
  blinkerPace = 0.5;
}

void blink() {
    Serial.println("Blink...");
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));
}

void toggle() {
    Serial.println("Toggle...");

  static bool isBlinking = false;
  if (isBlinking) {
    blinker.detach();
    isBlinking = false;
  }
  else {
    blinker.attach(blinkerPace, blink);
    isBlinking = true;
  }
  digitalWrite(LED_PIN, LOW);  //make sure LED on on after toggling (pin LOW = led ON)
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 1);
  delay(1000);
  digitalWrite(LED_PIN, 0);
  toggler.attach(togglePeriod, toggle);
  changer.once(30, change);
  delay(1000);
  Serial.begin(115200);
  Serial.println("starting...");
}

void loop() {
  
}
