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
#include <ESP8266mDNS.h>
#include <time.h>

const float GMT = 0;
const float UTC = 0;
const float ECT = 1.00;
const float EET = 2.00;
const float ART = 2.00;
const float EAT = 3.00;
const float MET = 3.30;
const float NET = 4.00;
const float PLT = 5.00;
const float IST = 5.30;
const float BST = 6.00;
const float VST = 7.00;
const float CTT = 8.00;
const float JST = 9.00;
const float ACT = 9.30;
const float AET = 10.00;
const float SST = 11.00;
const float NST = 12.00;
const float MIT = -11.00;
const float HST = -10.00;
const float AST = -9.00;
const float PST = -8.00;
const float PNT = -7.00;
const float MST = -7.00;
const float CST = -6.00;
const float EST = -5.00;
const float IET = -5.00;
const float PRT = -4.00;
const float CNT = -3.30;
const float AGT = -3.00;
const float BET = -3.00;
const float CAT = -1.00;

const int EPOCH_1_1_2019 = 1546300800; //1546300800 =  01/01/2019 @ 12:00am (UTC)

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

time_t now;

String localTimeString()
{

  time(&now);
  struct tm *timeinfo = localtime(&now);
  int year = timeinfo->tm_year + 1900;
  int month = timeinfo->tm_mon + 1;
  int day  = timeinfo->tm_mday + 1;
  int hour = timeinfo->tm_hour;
  int mins = timeinfo->tm_min;
  int sec = timeinfo->tm_sec;
  int day_of_week = timeinfo->tm_wday;
  
  return "Date = " + String(day) + "/" + String(month) + "/" + String(year) + " Time = " + String(hour) + ":" + String(mins) + ":" + String(sec);
  }

void handleRoot() {

  Serial.println(localTimeString());
  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! " + localTimeString());
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
  digitalWrite(led, LOW);
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  configTime(PST, 0, "pool.ntp.org", "time.nist.gov");


  while (now < EPOCH_1_1_2019)
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

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.getServer().setRSACert(new BearSSL::X509List(serverCert), new BearSSL::PrivateKey(serverKey));

  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");
}

void loop(void){
  server.handleClient();
  MDNS.update();
}
