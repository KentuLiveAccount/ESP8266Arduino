#include <ESP8266WiFi.h>

/*
 *   
 1. update network-name and pass-to-network
 2. open Com monitor select 115200 baud
 3. reset the device
 */
 
void setup()
{
  Serial.begin(115200);
  Serial.println();

  WiFi.begin("network-name", "pass-to-network");

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {}
