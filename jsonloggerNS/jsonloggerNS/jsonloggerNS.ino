
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


#define ServoPin 14   //D5 is GPIO14
#define DemultiplexPin 12 // D6 is GPIO12

#define TC_PIN A0          // set to ADC pin used
#define AREF 3.1159           // set to AREF, typically board voltage like 3.3 or 5.0
#define ADC_RESOLUTION 10  // set to ADC bit resolution, 10 is default

double V_0 = 3.291;

// WeMos D1 Mini internally contains voltage divider for A0 input in order to expand the voltage
// range of ESP8266 which is limited to 1.0V
// R_2 of the divider (between A0 and GND) is fixed 100k ohm
double R_2 = 100000.0; //VCC - R_1 - A0 - R_2 - GND (R_1 = 230 + rThermistor)

// D1 Mini also has 220k ohm on the VCC side of the divider
double R_1d = 220000.0;

// I'm adding additional 10k register on VCC side of the divider, along with the sensor
// to make sure input voltage to the A0 pin never exceed 1V.
double R_1Adj = 10000.0;


double factor = 10000000.0;
double CM0 = 9621.996628; //0.9621996628 e-3
double CM1 = 1800.586745; //1.800586745 e-4
double CM2 = 1.97132282; //1.971322820 e-7 

double sensorMax = 1.0;
Servo myservo;

IPAddress local_IP(192, 168, 1, 4);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

float get_voltage(int raw_adc) {
  float raw = raw_adc;
  float aref = AREF;
  float res = pow(2, ADC_RESOLUTION);
  
  return (raw * aref) / res;  
}
 
float get_temperature(float voltage) {
  return voltage * 200 - 250;
}

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
  pinMode(led, OUTPUT);
  pinMode(ServoPin, OUTPUT);
  pinMode(DemultiplexPin, OUTPUT);
  pinMode(A0, INPUT) ;
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  
  myservo.attach(ServoPin);
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

  server.on("/settemp", HTTP_POST, handleSetTemp);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);
}

float readThermoCouple()
{
    digitalWrite(DemultiplexPin, LOW);

    int reading = 0;
    for (int i = 0; i < 10; i++)
    {
      delay(10);
      reading = reading + analogRead(A0);
    }

    reading = reading / 10;

    Serial.println("\nreading: " + String(reading));
    float voltage = get_voltage(reading);
    Serial.println("\nvoltage: " + String(voltage));
    return get_temperature(voltage);
}

float readThermistor()
{
  digitalWrite(DemultiplexPin, HIGH);

  // read the pin value 10 times over 100 ms span and take average
  double sensorValue = 0;
  for (int i = 0; i < 10; i++)
  {
    delay(10);
    // read the input on analog pin 0:
    sensorValue += analogRead(A0);
  }

  sensorValue = sensorValue / 10;

  Serial.println("\nTermistorSensor: " + String(sensorValue ));

  // Convert the analog reading (which goes from 0 - 1023) to voltage reference (3.3V or 5V or other):
  double voltage = sensorValue * sensorMax /1023.0;

  // i'm adjusting the misterious temperature error (for now treating it as constant error)
  voltage = voltage - 0.028;

  double R_1 = R_2 * (V_0 - voltage)/voltage; // voltage to resistance

  // remove the fixed 230k ohm on the vcc side of the voltage dividor
  double rThermistor = R_1 - R_1d - R_1Adj;

  Serial.println("\nTermistorOhm: " + String(rThermistor));
  if (rThermistor < 0)
    return 0;

  //rThermistor = 133000;
  double lnR = log(rThermistor);

  // T(K) = 1/(C0 + C1Ln(R) + C2(Ln(R))^3)
  double temp  = factor /(CM0 + lnR * CM1 + CM2 * lnR * lnR * lnR);
  temp  = temp - 273.15; // K -> C

  if (temp > 300)
    temp = 0;

  return temp;
}

void loop(void){
  server.handleClient();
  int millisNow = millis();
  if (millisTimeLast == 0 || millisNow - millisTimeLast > (1000 * 10))
  {
    millisTimeLast = millisNow;
    int tempC  = readThermoCouple();
    Serial.println("\nX-tempC: " + String(tempC));

    currentTemp  = tempC * 9 / 5 + 32;
    Serial.println("\nX-tempF: " + String(currentTemp));

    tempC = readThermistor();
    Serial.println("\nI-tempC: " + String(tempC));

    currentInternalTemp  = tempC * 9 / 5 + 32;
    Serial.println("\nI-tempF: " + String(currentInternalTemp));
    AddTemp(currentTemp, currentInternalTemp);
    Serial.println("\ntemps: " + ReadTemps());
    

    myPID.Compute();

    int angleI = angle > 105 ? 105 : angle;
    angleI = angleI < 0 ? 0 : angleI;

    Serial.println("\nservo angle: " + String(angleI));

    myservo.write(angleI);

  }
}
