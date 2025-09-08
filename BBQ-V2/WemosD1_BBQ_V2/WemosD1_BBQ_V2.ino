
/*

  create a skeleton  code that has large upload and see if problem repeats.

  max content 901 characters, anything loger winds up with Recv isse even with static text.
  even with 901 characters, unknown lenght also results in recv
    // curl: (56) Recv failure: Connection was reset

  HelloServerBearSSL - Simple HTTPS server example
  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.
  Adapted by Earle F. Philhowe
  r, III, from the HelloServer.ino example.
  This example is released into the public domain.

  curl -X POST -H "Content-Type: text/plain" "http://192.168.1.4/settemp" -d "62"
  curl -X POST -H "Content-Type: text/plain" "http://192.168.1.4/setpidp" -d "0"
  curl -X POST -H "Content-Type: text/plain" "http://192.168.1.4/setpidi" -d "0"
  curl -X POST -H "Content-Type: text/plain" "http://192.168.1.4/setpidd" -d "0"
*/
//#include <WiFiManager.h>

#include "MCP3202.h"
#include "ThermistorMath.h"
#include "RunningAverage.h"
#include "DataLogger.h"
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <PID_v1.h> // from https://github.com/br3ttb/Arduino-PID-Library/
#include <math.h>
#include "wifisetting.h"

#define NTCMuxSelect D2
#define ResMuxSelect0 D0
#define ResMuxSelect1 D3
#define MCP3202_CS D8

#define MCP_TestIn 0
#define MCP_TempIn 1

#define ServoPWM D1
const int led = D4;

#define DEBUG(X) X
//#define DEBUG(X) ;

// other constants
double V_0 = 3.295; // 3.3v is the standard supply
double ResistorBank1 =  9951.5592; //supposed to be 10k.

double targetTemp = 205;
double currentTemp = 0;
double angle = 0;
double pidP = 13;
double pidI = 20;
double pidD = 15;

PID myPID(&currentTemp, &angle, &targetTemp, pidP, pidI, pidD, DIRECT);

MCP3202 mcp(MCP3202_CS, V_0);

Servo myservo;

ESP8266WebServer server(80);


void handleRoot() {

  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! ");
  delay(200);
  digitalWrite(led, LOW);
}



const char accessControlAllowOrigin[] = "Access-Control-Allow-Origin";
const char applicationJsonUtf8[] = "application/json;charset=utf-8";
void handleJson() 
{
  Serial.print("\nJson - Start: ");

  digitalWrite(led, HIGH);

  server.sendHeader(accessControlAllowOrigin,"*");
  server.sendHeader("Expires", "-1");
  
  server.chunkedResponseModeStart(200, applicationJsonUtf8);

  SendTemps<ESP8266WebServer>(server);

  server.chunkedResponseFinalize();

  delay(200);
  digitalWrite(led, LOW);
  Serial.println("Json - End");
}

void handleJsonPID()
{
  Serial.print("\nJsonPID - Start: ");

  digitalWrite(led, HIGH);

  server.sendHeader(accessControlAllowOrigin,"*");
  server.sendHeader("Expires", "-1");

  server.chunkedResponseModeStart(200, applicationJsonUtf8);

  SendTempsPID<ESP8266WebServer>(server);

  server.chunkedResponseFinalize();

  delay(200);
  digitalWrite(led, LOW);

  Serial.println("JsonPID - End");
}

void handleCur() {
  digitalWrite(led, HIGH);
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, applicationJsonUtf8, "{\"message\": [" + ReadCur() + "]}");
  delay(200);
  digitalWrite(led, LOW);  
}

void handleCurPID() {
  digitalWrite(led, HIGH);
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, applicationJsonUtf8, "{\"message\": [" + ReadCurPID() + "]}");
  delay(200);
  digitalWrite(led, LOW);  
}

void handleSetTemp() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost - SetTemp");

  Serial.println("arg: " + server.arg("plain"));
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, "text/plain", "temp set");
  targetTemp = server.arg("plain").toInt();
  UpdateCurrentTarget(targetTemp);

  delay(200);
  digitalWrite(led, LOW);
}

void handleSetPidP() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost - SetPidP");

  Serial.println("arg: " + server.arg("plain"));
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, "text/plain", "PidP updated");
  pidP = server.arg("plain").toDouble();
  myPID.SetTunings(pidP, pidI, pidD);

  delay(200);
  digitalWrite(led, LOW);
}

void handleSetPidI() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost - SetPidI");

  Serial.println("arg: " + server.arg("plain"));
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, "text/plain", "PidI updated");
  pidI = server.arg("plain").toDouble();
  myPID.SetTunings(pidP, pidI, pidD);

  delay(200);
  digitalWrite(led, LOW);
}

void handleSetPidD() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost - SetPidD");

  Serial.println("arg: " + server.arg("plain"));
  server.sendHeader(accessControlAllowOrigin,"*");
  server.send(200, "text/plain", "PidD updated");
  pidD = server.arg("plain").toDouble();
  myPID.SetTunings(pidP, pidI, pidD);

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

  mcp.Initialize();

  pinMode(NTCMuxSelect, OUTPUT);
  digitalWrite(NTCMuxSelect, LOW);

  pinMode(ServoPWM, OUTPUT);
  Serial.begin(115200);

  Serial.print("setup started\n");
  
  myservo.attach(ServoPWM);
  myservo.write(0);
  delay(1000);
  myservo.write(100);
  delay(1000);
  myservo.write(0);

  myPID.SetMode(AUTOMATIC);

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  WiFi.config(IPAddress(10,0,0,10)  /*ip*/, IPAddress(10,0,0,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/));
  //WiFi.config(IPAddress(192,168,1,4)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/));
  WiFi.begin(WIFINAME, WIFIPW);

  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
//  Serial.print("Connected to ");
//  Serial.println(wm.getWiFiSSID(false /*persistent*/));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/json", HTTP_GET, handleJson);
  server.on("/jsonpid", HTTP_GET, handleJsonPID);

  server.on("/cur", HTTP_GET, handleCur);
  server.on("/curpid", HTTP_GET, handleCurPID);

  server.on("/settemp", HTTP_POST, handleSetTemp);

  server.on("/setpidp", HTTP_POST, handleSetPidP);
  server.on("/setpidi", HTTP_POST, handleSetPidI);
  server.on("/setpidd", HTTP_POST, handleSetPidD);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);
}

#define amb 0
#define meat 1

Thermistor::ThermistorValues tv[2] = {
  {
    0, //channel zero is ambient
    5022.961427, //0.0005022961427
    2484.745814, // 0.0002484745814
    -0.06444353066 //-0.000000006444353066 * 10 ^ 7
  },
  {
    1, //channel one is meat
    7515.308604, //0.0007515308604
    2108.767213, //0.0002108767213
    1.118860294 //0.0000001118860294
  }
};

float CelciousFromAdc(double adcIn, int sensor)
{
  double rThermistor = mcp.ohmFromADC(adcIn, ResistorBank1);

  //DEBUG(Serial.println("\nTermistorOhm: " + String(rThermistor)));
  return Thermistor::CelciusFromOhm(rThermistor, tv[sensor]);
}

double minmax(double val, double mn, double mx)
{
  if (isnan(val))
    return mn;
  return val < mn ? mn : (val > mx ? mx : val);
}

NestedRunningAverage<50, 20> avg[2];
int millisTimeLast = 0;

void loop(void)
{
  //Serial.print(".");
  server.handleClient();

  int channel0 = mcp.Read(MCP_TempIn);

  digitalWrite(NTCMuxSelect, tv[meat].pin);
  delay(5);

  int channel1 = mcp.Read(MCP_TempIn);
  digitalWrite(NTCMuxSelect, tv[amb].pin);

  avg[0].Add(channel0);
  avg[1].Add(channel1);

  double tempCAmb = minmax(CelciousFromAdc(avg[0].Average(), amb), 0.0, 400.0);
  double tempCMeat= minmax(CelciousFromAdc(avg[1].Average(), meat), 0.0, 400.0);

  currentTemp  = Thermistor::farenheightFromCelsius(tempCAmb);
  double currentInternalTemp  = Thermistor::farenheightFromCelsius(tempCMeat);

  myPID.Compute();

  int millisNow = millis();
  if (millisTimeLast != 0 && abs(millisNow - millisTimeLast) < (1000 * 10))
    return;

  //DEBUG(Serial.println("\nchan0: " + String(channel0)));
  //DEBUG(Serial.println("\nchan1: " + String(channel1)));

  millisTimeLast = millisNow;


  int angleI = floor(minmax(angle, 0.0, 255.0));
  
  angleI = angleI * 105 / 255;

  AddTemp(currentTemp, currentInternalTemp, targetTemp, angleI, pidP,  pidI, pidD);

  //DEBUG(Serial.println("\nservo angle: " + String(angle)));
  //DEBUG(Serial.println("\nservo angleI: " + String(angleI)));
  DEBUG(Serial.printf("%f, %f, %d\n", currentTemp, currentInternalTemp, angleI));

  myservo.write(angleI);
}
