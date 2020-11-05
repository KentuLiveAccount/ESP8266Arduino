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
#include <ESP8266WebServerSecure.h>


#ifndef STASSID
#define STASSID "----"
#define STAPSK  "----"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

BearSSL::ESP8266WebServerSecure server(443);

static const char serverCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
-----END CERTIFICATE-----
)EOF";

static const char serverKey[] PROGMEM =  R"EOF(
-----BEGIN PRIVATE KEY-----
-----END PRIVATE KEY-----
)EOF";


const int led = 2;

void handleRoot() {

  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! ");
  delay(200);
  digitalWrite(led, LOW);
}

void handleJson() {

  digitalWrite(led, HIGH);
  server.send(200, "application/json;charset=utf-8", "{\"message\": \"hello from ESP8266!\"}");
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

  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  time_t now;

  while (now < 1546300800)
  {
    now = time(nullptr);
    delay(500);
    Serial.print("*");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.enableCORS(true);

  server.on("/", handleRoot);

  server.on("/json", handleJson);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);

}

void loop(void){
  server.handleClient();
}
