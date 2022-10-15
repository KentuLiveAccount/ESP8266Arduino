
/*
  HelloServerBearSSL - Simple HTTPS server example
  This example demonstrates a basic ESP8266WebServerSecure HTTPS server
  that can serve "/" and "/inline" and generate detailed 404 (not found)
  HTTP respoinses.  Be sure to update the SSID and PASSWORD before running
  to allow connection to your WiFi network.
  Adapted by Earle F. Philhowe
  r, III, from the HelloServer.ino example.
  This example is released into the public domain.
*/
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <PID_v1.h>
#include <math.h>
#include "wifisetting.h"
#include <SPI.h>



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


const int cTempMax = 100;
int temps[cTempMax];
int intTemps[cTempMax];
int target[cTempMax];
int cTemp = 0;
int millisTimeLast = 0;
double targetTemp = 100;
double currentTemp = 0;
double currentInternalTemp = 0;
double angle = 0;

PID myPID(&currentTemp, &angle, &targetTemp,2,5,1, DIRECT);

const int led = 2;

void AddTemp(int temp, int tempInt)
{
  temps[(cTemp % cTempMax)] = temp;
  intTemps[cTemp % cTempMax] = tempInt;
  target[(cTemp++ % cTempMax)] = targetTemp;
}

void UpdateCurrentTarget(int newTarget)
{
  target[(cTemp + cTempMax - 1) % cTempMax] = newTarget;
}

String ReadCur()
{
  const int i = (cTemp + cTempMax - 1) % cTempMax;

  String str = "";
  str += "{ \"currenttemp\": ";
  str += String(temps[i]);
  str += ", \"internatemp\": ";
  str += String(intTemps[i]);
  str += ", \"targettemp\": ";
  str += String(target[i]);
  str += ", \"servoangle\": ";
  str += String(angle);
  str += "}";

  return str;
}

String ReadTemps()
{
  int count = cTemp < cTempMax ? cTemp : cTempMax;
  int i = cTemp < cTempMax ? 0 : cTemp;

  String str = "";
  while (count)
  {
    str += "{ \"currenttemp\": ";
    str += String(temps[i % cTempMax]);
    str += ", \"internatemp\": ";
    str += String(intTemps[i % cTempMax]);
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

void handleCur() {
  digitalWrite(led, HIGH);
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "application/json;charset=utf-8", "{\"message\": [" + ReadCur() + "]}");
  delay(200);
  digitalWrite(led, LOW);  
}

void handleSetTemp() {
  digitalWrite(led, HIGH);
  Serial.println("gotPost");

  Serial.println("arg: " + server.arg("plain"));
  server.sendHeader("Access-Control-Allow-Origin","*");
  server.send(200, "text/plain", "temp set");
  targetTemp = server.arg("plain").toInt();
  UpdateCurrentTarget(targetTemp);

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

  pinMode(CS_3202, OUTPUT);
  digitalWrite(CS_3202, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);

  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  pinMode(ServoPin, OUTPUT);
  Serial.begin(115200);
  
  myservo.attach(ServoPin);
  myservo.write(0);
  delay(1000);
  myservo.write(100);
  delay(1000);
  myservo.write(0);

  myPID.SetMode(AUTOMATIC);

   WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wm;

  // reset settings - wipe stored credentials for testing
  // these are stored by the esp library
  //wm.resetSettings();

  // Automatically connect using saved credentials,
  // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
  // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
  // then goes into a blocking loop awaiting configuration and will return success result

  wm.setSTAStaticIPConfig(IPAddress(192,168,1,4)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/)); // optional DNS 4th argument

  bool res;
  res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if(!res) {
      Serial.println("Failed to connect");
      // ESP.restart();
  } 
  else {
      //if you get here you have connected to the WiFi    
      Serial.println("connected...yeey :)");
  }

  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(wm.getWiFiSSID(false /*persistent*/));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);

  server.on("/json", HTTP_GET, handleJson);

  server.on("/cur", HTTP_GET, handleCur);

  server.on("/settemp", HTTP_POST, handleSetTemp);

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

  if (temp > 500)
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

    AddTemp(rThermistorAmb, rThermistorMeat);

    numberofsamples = 0;
    channel0accumulator = 0;
    channel1accumulator = 0;
  }
}
#else
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
    double tempCAmb = CelciousFromAdc(channel0avg, amb);

    double tempCMeat= CelciousFromAdc(channel1avg, meat);

    millisTimeLast = millisNow;
    DEBUG(Serial.println("\nX-tempC: " + String(tempCAmb)));

    currentTemp  = farenheightFromCelsius(tempCAmb);
    DEBUG(Serial.println("\nX-tempF: " + String(currentTemp)));

    DEBUG(Serial.println("\nI-tempC: " + String(tempCMeat)));

    currentInternalTemp  = farenheightFromCelsius(tempCMeat);
    DEBUG(Serial.println("\nI-tempF: " + String(currentInternalTemp)));
    AddTemp(currentTemp, currentInternalTemp);
    //Serial.println("\ntemps: " + ReadTemps());

    numberofsamples = 0;
    channel0accumulator = 0;
    channel1accumulator = 0;

    myPID.Compute();

    int angleI = angle > 105 ? 105 : angle;
    angleI = angleI < 0 ? 0 : angleI;

    DEBUG(Serial.println("\nservo angle: " + String(angleI)));

    myservo.write(angleI);

  }
}
#endif