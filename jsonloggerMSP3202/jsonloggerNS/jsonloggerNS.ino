
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

#define LWIP_TCP_CLOSE_TIMEOUT_MS_DEFAULT 400000
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <PID_v1.h> // from https://github.com/br3ttb/Arduino-PID-Library/
#include <math.h>
#include "wifisetting.h"
#include <SPI.h>
#include "EspMQTTClient.h"

EspMQTTClient mqttClient(
  WIFINAME,
  WIFIPW,
  "192.168.1.100",  // MQTT Broker server ip
  "BBQ_ESP8266-1"      // Client name that uniquely identify your device
);




#define ServoPin 5   //D1 is GPIO5
#define CS_3202 15   //D8 is GPIO15

#define AREF 3.3           // set to AREF, typically board voltage like 3.3 or 5.0
#define ADC_RESOLUTION 12  // set to ADC bit resolution, 10 is default

#define DEBUG(X) X
//#define DEBUG(X) ;

double V_0 = 3.3; // 3.3v is the standard supply

double R_2 = 2001.0; //Fixed register closer to ground. 2.0k but the one had error

double factor = 10000000.0;

double sensorMax = 1.0;
Servo myservo;

ESP8266WebServer server(80);


const int cDataMax = 100;
int temps[cDataMax];
int intTemps[cDataMax];
int target[cDataMax];
int angles[cDataMax];
double pidPs[cDataMax];
double pidIs[cDataMax];
double pidDs[cDataMax];
int cData = 0;
int millisTimeLast = 0;
double targetTemp = 205;
double currentTemp = 0;
double currentInternalTemp = 0;
double angle = 0;
double pidP = 13;
double pidI = 20;
double pidD = 15;

PID myPID(&currentTemp, &angle, &targetTemp, pidP, pidI, pidD, DIRECT);

const int led = 2;

String s_string;

//required for some reason...
void onConnectionEstablished() {
  Serial.print("Wifi, mqtt connected!");
}

void ReadTempsPID();

void AddTemp(int temp, int tempInt, int angleI)
{
  const int i = cData++ % cDataMax;
  temps[i] = temp;
  intTemps[i] = tempInt;
  target[i] = targetTemp;
  angles[i] = angleI;
  pidPs[i] = pidP;
  pidIs[i] = pidI;
  pidDs[i] = pidD;

  //ReadTempsPID();
}

int currentIndex()
{
  return (cData + cDataMax - 1) % cDataMax;
}

void UpdateCurrentTarget(int newTarget)
{
  target[currentIndex()] = newTarget;
}

String ReadCur()
{
  const int i = currentIndex();

  String str = "";
  str += "{ \"currenttemp\": ";
  str += String(temps[i]);
  str += ", \"internatemp\": ";
  str += String(intTemps[i]);
  str += ", \"targettemp\": ";
  str += String(target[i]);
  str += ", \"servoangle\": ";
  str += String(angles[i]);
  str += "}";

  return str;
}

String ReadCurPID()
{
  const int i = currentIndex();

  String str = "{ \"currenttemp\": ";
  str += String(temps[i]);
  str += ", \"internatemp\": ";
  str += String(intTemps[i]);
  str += ", \"targettemp\": ";
  str += String(target[i]);
  str += ", \"servoangle\": ";
  str += String(angles[i]);
  str += ", \"pidp\": ";
  str += String(pidPs[i]);
  str += ", \"pidi\": ";
  str += String(pidIs[i]);
  str += ", \"pidd\": ";
  str += String(pidDs[i]);
  str += "}";

  return str;
}

String ReadTemps()
{
  int count = cData < cDataMax ? cData : cDataMax;
  int i = cData < cDataMax ? 0 : cData;

  String str = "";
  while (count)
  {
    str += "{ \"currenttemp\": ";
    str += String(temps[i % cDataMax]);
    str += ", \"internatemp\": ";
    str += String(intTemps[i % cDataMax]);
    str += ", \"targettemp\": ";
    str += String(target[i++ % cDataMax]);
    str += "}";

    if (count > 1)
      str += ", ";
    count--;
  }

  return str;
}

/*
json
 13  {"message": [
60  {"currenttemp": 000, "internatemp": 000, "targettemp": 000},
  2  ]}

jsonpid
 13  {"message": [
119  {"currenttemp": 000, "internatemp": 000, "targettemp": 000, "angle": 000, "pidP": 00.00, "pidI": 00.00, "pidD": 00.00},
  2  ]}

  11900 + 15 = 11,915 bytes
*/
void ReadTempsPID()
{
  int count = cData < cDataMax ? cData : cDataMax;
  int i = cData < cDataMax ? 0 : cData;

  s_string = "{\"message\": [";
  while (count)
  {
    int t = i++ % cDataMax;
    s_string += "{ \"currenttemp\": ";
    s_string += String(temps[t]);
    s_string += ", \"internatemp\": ";
    s_string += String(intTemps[t]);
    s_string += ", \"targettemp\": ";
    s_string += String(target[t]);
    s_string += ", \"angle\": ";
    s_string += String(angles[t]);
    s_string += ", \"pidP\": ";
    s_string += String(pidPs[t]);
    s_string += ", \"pidI\": ";
    s_string += String(pidIs[t]);
    s_string += ", \"pidD\": ";
    s_string += String(pidDs[t]);
    s_string += "}";

    if (count > 1)
      s_string += ", ";
    count--;
  }

  s_string += "]}\n";
}

void handleRoot() {

  digitalWrite(led, HIGH);
  server.send(200, "text/plain", "Hello from esp8266 over HTTPS! ");
  delay(200);
  digitalWrite(led, LOW);
}

void  SendTemps()
{
  int count = cData < cDataMax ? cData : cDataMax;
  int i = cData < cDataMax ? 0 : cData;

  while (count)
  {
    server.sendContent("{ \"currenttemp\": ");
    server.sendContent(String(temps[i % cDataMax]));
    server.sendContent(", \"internatemp\": ");
    server.sendContent(String(intTemps[i % cDataMax]));
    server.sendContent(", \"targettemp\": ");
    server.sendContent(String(target[i++ % cDataMax]));

    if (count > 1)
      server.sendContent("}, ");
    else
      server.sendContent("}");
    count--;
  }
}

const char accessControlAllowOrigin[] = "Access-Control-Allow-Origin";
const char applicationJsonUtf8[] = "application/json;charset=utf-8";
void handleJson() 
{
  Serial.print("\nJson - Start: ");

  digitalWrite(led, HIGH);

  server.sendHeader(accessControlAllowOrigin,"*");
  server.sendHeader("Expires", "-1");
  //server.setContentLength(961);
  
  int count = 100; //15 (max 100)

  server.chunkedResponseModeStart(200, applicationJsonUtf8);
  server.sendContent("{\"message\": [");
  for (int i = 0; i < (count);i++)
  {
    if (i < (count - 1))
      server.sendContent("{ \"currenttemp\": 32, \"internatemp\": 32, \"targettemp\": 205},");
    else
      server.sendContent("{ \"currenttemp\": 32, \"internatemp\": 32, \"targettemp\": 205}]}");
  }

  server.chunkedResponseFinalize();

  //SendTemps();
  Serial.println("Json - Stopping");
  //server.client().stop(40000);
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
  //server.setContentLength(17);
  server.chunkedResponseModeStart(200, applicationJsonUtf8);

  server.sendContent("{\"message\": [");
  server.sendContent("{ \"currenttemp\": 32, \"internatemp\": 32, \"targettemp\": 205},");
  server.sendContent("{ \"currenttemp\": 32, \"internatemp\": 32, \"targettemp\": 205}]}");

  server.chunkedResponseFinalize();

  //Serial.println("JsonPid - Stopping");
  //server.client().stop(40000);

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

  s_string.reserve(20000);

  pinMode(CS_3202, OUTPUT);
  digitalWrite(CS_3202, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  pinMode(ServoPin, OUTPUT);
  Serial.begin(115200);

  Serial.print("setup started\n");
  
  myservo.attach(ServoPin);
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

  #if 0
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  //WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  //wm.setSTAStaticIPConfig(IPAddress(192,168,1,4)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/)); // optional DNS 4th argument
  ////wm.setSTAStaticIPConfig(IPAddress(192,168,1,12)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/)); // optional DNS 4th argument

  //bool res;
  //res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  //if(!res) {
  //    Serial.println("Failed to connect");
      // ESP.restart();
  //} 
  //else {
      //if you get here you have connected to the WiFi    
  //    Serial.println("connected...yeey :)");
  //}
  #endif

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

int Read3202(int CHANNEL) {
  int msb;
  int lsb;
  int commandBytes = B10100000 ;// channel 0
  if (CHANNEL == 1) 
    commandBytes = B11100000 ; // channel 1

  digitalWrite(CS_3202, LOW);
  delay(2);

  SPI.transfer (B00000001);// Start bit
  msb = SPI.transfer(commandBytes) ;
  msb = msb & B00001111;
  lsb = SPI.transfer(0x00);
  digitalWrite(CS_3202, HIGH);

  return ((int) msb) << 8 | lsb;
}

double ohmFromADC(double adcIn)
{
  double sensorMax = pow(2, ADC_RESOLUTION);
  // Convert the analog reading (which goes from 0 - 4096) to voltage reference (3.3V):
  double voltage = adcIn * 3.3  / sensorMax;

  double R_1 = 0;

  if (voltage > 0)
    R_1 = R_2 * (V_0 - voltage)/voltage; // voltage to resistance

  return R_1;
}

struct ThermistorValues {
  int pin;
  double CM0;
  double CM1;
  double CM2;
};

#define amb 0
#define meat 1

ThermistorValues tv[2] = {
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

float CelciusFromOhm(double ohm, int sensor)
{
  if (ohm < 0)
    return 0;

  double lnR = log(ohm);

  // T(K) = 1/(C0 + C1Ln(R) + C2(Ln(R))^3)
  double temp  = factor /(tv[sensor].CM0 + lnR * tv[sensor].CM1 + tv[sensor].CM2 * lnR * lnR * lnR);
  temp  = temp - 273.15; // K -> C

  if (temp > 500 || temp < 0)
    temp = 0;

  return temp;
}

float CelciousFromAdc(double adcIn, int sensor)
{
  double rThermistor = ohmFromADC(adcIn);

  DEBUG(Serial.println("\nTermistorOhm: " + String(rThermistor)));
  return CelciusFromOhm(rThermistor, sensor);
}

float farenheightFromCelsius(double c)
{
    return c * 9 / 5 + 32;
}

long channel0accumulator = 0;
long channel1accumulator = 0;
int numberofsamples = 0;
double channel0avg = 0.0;
double channel1avg = 0.0;

//#define THERMISTORCALIBRATION 1

#ifdef THERMISTORCALIBRATION
void loop(void){
  server.handleClient();
  int channel0 = Read3202(tv[amb].pin);
  int channel1 = Read3202(tv[meat].pin);

  if (numberofsamples < 200)
  {
    channel0accumulator = channel0accumulator + channel0;
    channel1accumulator = channel1accumulator + channel1;
    numberofsamples++;
  }

  int millisNow = millis();
  if (millisTimeLast == 0 || abs(millisNow - millisTimeLast) > (1000 * 10))
  {
    channel0avg = (double)channel0accumulator / (double)numberofsamples;
    channel1avg = (double)channel1accumulator / (double)numberofsamples;

    millisTimeLast = millisNow;
    double rThermistorAmb = ohmFromADC(channel0avg);

    double rThermistorMeat = ohmFromADC(channel1avg);

    AddTemp(rThermistorAmb, rThermistorMeat, 0 /* angleI */);

    numberofsamples = 0;
    channel0accumulator = 0;
    channel1accumulator = 0;
  }

}
#else

double minmax(double val, double mn, double mx)
{
  if (isnan(val))
    return mn;
  return val < mn ? mn : (val > mx ? mx : val);
}

void loop(void)
{
  //Serial.print(".");
  server.handleClient();
  int channel0 = Read3202(tv[amb].pin);
  int channel1 = Read3202(tv[meat].pin);

  if (numberofsamples < 200)
  {
    channel0accumulator = channel0accumulator + channel0;
    channel1accumulator = channel1accumulator + channel1;
    numberofsamples++;
  }

  int millisNow = millis();
  if (millisTimeLast == 0 || abs(millisNow - millisTimeLast) > (1000 * 10))
  {
    DEBUG(Serial.println("\nchan0: " + String(channel0)));
    DEBUG(Serial.println("\nchan1: " + String(channel1)));
  
    channel0avg = (double)channel0accumulator / (double)numberofsamples;
    channel1avg = (double)channel1accumulator / (double)numberofsamples;

    millisTimeLast = millisNow;
    double tempCAmb = minmax(CelciousFromAdc(channel0avg, amb), 0.0, 400.0);

    double tempCMeat= minmax(CelciousFromAdc(channel1avg, meat), 0.0, 400.0);

    millisTimeLast = millisNow;
    DEBUG(Serial.println("\nX-tempC: " + String(tempCAmb)));

    currentTemp  = farenheightFromCelsius(tempCAmb);
    DEBUG(Serial.println("\nX-tempF: " + String(currentTemp)));

    DEBUG(Serial.println("\nI-tempC: " + String(tempCMeat)));

    currentInternalTemp  = farenheightFromCelsius(tempCMeat);
    DEBUG(Serial.println("\nI-tempF: " + String(currentInternalTemp)));
    //Serial.println("\ntemps: " + ReadTemps());

    numberofsamples = 0;
    channel0accumulator = 0;
    channel1accumulator = 0;

    myPID.Compute();

    int angleI = floor(minmax(angle, 0.0, 255.0));
    
    angleI = angleI * 105 / 255;

    AddTemp(currentTemp, currentInternalTemp, angleI);

    DEBUG(Serial.println("\nservo angle: " + String(angle)));
    DEBUG(Serial.println("\nservo angleI: " + String(angleI)));

    myservo.write(angleI);

    mqttClient.publish("bbqtemp/internalTemp", String(intTemps[currentIndex()]));
    mqttClient.publish("bbqtemp/externalTemp", String(temps[currentIndex()]));
    mqttClient.publish("test/esptest", String(currentIndex()));
  }

  mqttClient.loop();
}
#endif
