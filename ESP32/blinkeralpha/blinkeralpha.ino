
#include <WiFi.h>

void setup() {
  pinMode(2, OUTPUT);

  Serial.begin(115200);
  Serial.println("Serial port enabled");

  WiFi.begin("Bearsden 2.0", "Peace10310313");
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }

}

void loop() {
  Serial.println("\nConnected to the WiFi network");
  Serial.print("Local ESP32 IP: ");
  Serial.println(WiFi.localIP());

  int val = 0;
  val += analogRead(0);
  val += analogRead(0);
  val += analogRead(0);
  val += analogRead(0);
  val += analogRead(0);
  val = val / 5;
  Serial.print("Analog read value:");
  Serial.println(val);

  int wait = (900 * val / 4096) + 100;
  digitalWrite(2, HIGH);
  delay(wait);
  digitalWrite(2, LOW);
  delay(wait);
}
