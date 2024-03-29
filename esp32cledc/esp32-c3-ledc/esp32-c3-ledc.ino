const int ledPin = 2; //4 works too!
 
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
