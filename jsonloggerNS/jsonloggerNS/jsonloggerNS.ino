/*
  HelloServerBearSSL - Simple HTTPS server example
  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.
  Adapted by Earle F. Philhower, III, from the HelloServer.ino example.
  This example is released into the public domain.
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


#ifndef STASSID
#define STASSID "----"
#define STAPSK  "----"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

const int cTempMax = 100;
int temps[cTempMax];
int target[cTempMax];
int cTemp = 0;
int millisTimeLast = 0;
int targetTemp = 100;

const int led = 2;

void AddTemp(int temp)
{
  temps[(cTemp % cTempMax)] = temp;
  target[(cTemp++ % cTempMax)] = targetTemp;
}

String ReadTemps()
{
  int count = cTemp < cTempMax ? cTemp : cTempMax;
  int i = cTemp < cTempMax ? 0 : cTemp + 1;

  String str = "";
  while (count)
  {
    str += "{ \"currenttemp\": ";
    str += String(temps[i % cTempMax]);
    str += ", \"targettemp\": ";
    str += String(target[i++ % cTempMax]);
    str += "}";

    if (count > 1)
      str += ", ";
    count--;
  }

  return str;
}

void handleRoot() {

  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! ");
  delay(200);
  digitalWrite(led, LOW);
}

void handleJson() {

  digitalWrite(led, HIGH);
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "application/json;charset=utf-8", "{\"message\": [" + ReadTemps() + "]}");
  delay(200);
  digitalWrite(led, LOW);
}

void handleSetTemp() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost");

  Serial.println("arg: " + server.arg("plain"));
  server.send(200, "text/plain", "temp set");
  targetTemp = server.arg("plain").toInt();
  delay(200);
  digitalWrite(led, LOW);

}

void handleNotFound(){
  digitalWrite(led, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, LOW);
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/json", HTTP_GET, handleJson);

  server.on("/settemp", HTTP_POST, handleSetTemp);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);
}

void loop(void){
  server.handleClient();
  int millisNow = millis();
  if (millisTimeLast == 0 || millisNow - millisTimeLast > (1000 * 5))
  {
    millisTimeLast = millisNow;
    int temp = analogRead(A0);
    Serial.println("\ntemp: " + String(temp));
    AddTemp(temp);
    Serial.println("\ntemps: " + ReadTemps());
  }
}
