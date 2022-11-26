#include <ESP8266WiFi.h>

void setup(){
  Serial.begin(115200);
  Serial.print("\nESP8266 MAC ADDR:  ");
  Serial.println(WiFi.macAddress());
}
 
void loop(){

}   
