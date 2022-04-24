#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RST 16
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RST);
const char* ssid = "Bearsden 2.0";
const char* pass = "Peace10310313";

const char* turn_on = "http://192.168.1.4/cur";

void setup() {
  // put your setup code here, to run once:
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  pinMode(16, OUTPUT);
  //Serial.begin(115200);
  digitalWrite(16, LOW);
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);
  Wire.begin(4, 15);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.setCursor(0, 0);
  display.print("BBQ Mon");
  display.display();
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:

  //getstrin is the url that it gets
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    http.begin(client, turn_on);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      StaticJsonDocument<192> doc;

      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());

        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(2);
        display.print("JSON Err: ");
        display.print(error.c_str());
        display.display();

        return;
      }

      JsonObject message_0 = doc["message"][0];
      int message_0_currenttemp = message_0["currenttemp"]; // 222
      int message_0_internatemp = message_0["internatemp"]; // 221
      int message_0_targettemp = message_0["targettemp"]; // 100
      int message_0_servoangle = message_0["servoangle"]; // 100
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2);
      display.print("Atm: ");
      display.print(message_0_currenttemp);
      display.setCursor(0,17);
      display.print("Int: ");
      display.print(message_0_internatemp);
      display.setCursor(0,34);
      display.print("Tgt: ");
      display.print(message_0_targettemp);
      display.setCursor(0,51);
      display.print("Srv: ");
      display.print(message_0_servoangle);
      display.display();
      
      Serial.println(payload);
    }
    else {
      display.clearDisplay();
      display.setCursor(0,0);
      display.setTextSize(2);
      display.print("HttpGet Err: ");
      display.print(String(httpResponseCode));
      display.display();

      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }

delay(10000);

}
