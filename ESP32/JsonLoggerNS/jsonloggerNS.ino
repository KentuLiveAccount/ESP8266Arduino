//#include <WiFiManager.h>

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//#include <Servo.h>
//#include <ESP32Servo.h>

#include <PID_v1.h>
#include <math.h>

#define led 5 // GPIO5 for LED
#define Thermistor_PIN 0          // set to ADC pin used -> GPIO0
#define Thermocouple_PIN 1        // set to ADC pin used -> GPIO1
#define ServoPin 4   //4 is GPIO4

#define ADC_RESOLUTION 12  // set to ADC bit resolution, 10 is default

double V_0 = 3.3; // 3.3v is the standard supply

double R_2 = 2001.0; //Fixed register closer to ground. 2.2k but the one had error

double factor = 10000000.0;

double CM0 = 9159.518265; // 0.9159518265 e-3 = 0.0009159518265
double CM1 = 1857.594704; //1.857594704 e-4 = 0.0001857594704
double CM2 = 1.843595405; //1.843595405 e-7 = 0.0000001843595405

class Servo {
    // Default min/max pulse widths (in microseconds) and angles (in
    // degrees).  Values chosen for Arduino compatibility.  These values
    // are part of the public API; DO NOT CHANGE THEM.
    static const int MIN_ANGLE = 0;
    static const int MAX_ANGLE = 180;
    
    static const int CHANNEL_MAX_NUM = 16;

public:
    static const int CHANNEL_NOT_ATTACHED = -1;

    // Pin number of unattached pins
    static const int PIN_NOT_ATTACHED = -1;
    
    /**
     * @brief Construct a new Servo instance.
     *
     * The new instance will not be attached to any pin.
     */
    Servo();

    /**
     * @brief Destruct a Servo instance.
     *     /**
     * @brief Associate this instance with a servomotor whose input is
     *        connected to pin.
     * @param pin Pin connected to the servo pulse wave input. This
     *            pin must be capable of PWM output (all ESP32 pins).
     *
     * @param channel Channel which is set to ESP32 Arduino function ledcSetup().
     *                Channel must be number between 0 - 15.
     *                It is possible to use automatic channel setup with constant
     *                Servo::CHANNEL_NOT_ATTACHED.
     * 
     * @param minAngle Target angle (in degrees) associated with
     *                 minPulseWidth.  Defaults to
     *                 MIN_ANGLE = 0.

     * Call _() and detach().
     */
    ~Servo();

     /*
     * @param maxAngle Target angle (in degrees) associated with
     *                 maxPulseWidth.  Defaults to
     *                 MAX_ANGLE = 180.
     *  
     * @param minPulseWidth Minimum pulse width to write to pin, in
     *                      microseconds.  This will be associated
     *                      with a minAngle degree angle.  Defaults to
     *                      MIN_PULSE_WIDTH = 544.
     *
     * @param maxPulseWidth Maximum pulse width to write to pin, in
     *                      microseconds.  This will be associated
     *                      with a maxAngle degree angle. Defaults to
     *                      MAX_PULSE_WIDTH = 2400.
     *
     * @sideeffect May set pinMode(pin, PWM).
     *
     * @return true if successful, false when pin doesn't support PWM.
     */
    bool attach(int pin, int channel = CHANNEL_NOT_ATTACHED, 
                int minAngle = MIN_ANGLE, int maxAngle = MAX_ANGLE);

    /**
     * @brief Stop driving the servo pulse train.
     *
     * If not currently attached to a motor, this function has no effect.
     *
     * @return true if this call did anything, false otherwise.
     */
    bool detach();

    /**
     * @brief Set the servomotor target angle.
     *
     * @param angle Target angle, in degrees.  If the target angle is
     *              outside the range specified at attach() time, it
     *              will be clamped to lie in that range.
     *
     * @see Servo::attach()
     */
    void write(int degrees);

    bool attached() const;

    /**
     * @brief Get the pin this instance is attached to.
     * @return Pin number if currently attached to a pin, PIN_NOT_ATTACHED
     *         otherwise.
     * @see Servo::attach()
     */
    int attachedPin() const;


private:
    void _resetFields(void);
    static int channel_next_free;

    int _pin;
    int _pulseWidthDuty;
    int _channel;
    int _min, _max;
    int _minAngle, _maxAngle;
};

int Servo::channel_next_free = 0;

Servo::Servo() {
    _resetFields();
};

Servo::~Servo() {
    detach();
}

bool Servo::attach(int pin, int channel, 
                   int minAngle, int maxAngle) 
{
    if(channel == CHANNEL_NOT_ATTACHED) {
        if(channel_next_free == CHANNEL_MAX_NUM) {
            return false;
        }
        _channel = channel_next_free;
        channel_next_free++;
    } else {
        _channel = channel;
    }

    _pin = pin;
    _minAngle = minAngle;
    _maxAngle = maxAngle;

    ledcSetup(_channel, 200, 8); // channel X, 200 Hz, 8-bit depth
    ledcAttachPin(_pin, _channel);
    return true;
}


bool Servo::detach() {
    if (!this->attached()) {
        return false;
    }

    if(_channel == (channel_next_free - 1))
        channel_next_free--;

    ledcDetachPin(_pin);
    _pin = PIN_NOT_ATTACHED;
    return true;
}

void Servo::write(int degrees) {
    ledcWrite(_channel, 25 + (130 - 25) * degrees / 180);
}

bool Servo::attached() const { return _pin != PIN_NOT_ATTACHED; }

int Servo::attachedPin() const { return _pin; }

void Servo::_resetFields(void) {
    _pin = PIN_NOT_ATTACHED;
    _pulseWidthDuty = 0;
    _channel = CHANNEL_NOT_ATTACHED;
    _minAngle = MIN_ANGLE;
    _maxAngle = MAX_ANGLE;
}

double sensorMax = 3.3;
Servo myservo; 

AsyncWebServer server(80);

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

void handleRoot(AsyncWebServerRequest *request) {

  digitalWrite(led, HIGH);
  request->send(200, "text/plain", "Hello from ESP32-C3 over HTTP! ");
  delay(200);
  digitalWrite(led, LOW);
}

void handleJson(AsyncWebServerRequest *request) {

  digitalWrite(led, HIGH);

  AsyncWebServerResponse *response = request->beginResponse(200, "application/json;charset=utf-8", "{\"message\": [" + ReadTemps() + "]}");
  response->addHeader("Access-Control-Allow-Origin","*");
  request->send(response);

  delay(200);
  digitalWrite(led, LOW);
}

void handleSetTemp(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
  digitalWrite(led, HIGH);
  Serial.println("gotPost");

  int temp = atoi((const char *)data);
  Serial.println(String("arg: ") + String(temp));

  AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "temp set");
  response->addHeader("Access-Control-Allow-Origin","*");
  request->send(response);
  targetTemp = temp;
  UpdateCurrentTarget(targetTemp);

  delay(200);
  digitalWrite(led, LOW);
}

void handleNotFound(AsyncWebServerRequest *request){
  digitalWrite(led, HIGH);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += request->url();
  message += "\nMethod: ";
  message += (request->method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += request->args();
  message += "\n";

  for (uint8_t i=0; i<request->args(); i++){
    message += " " + request->argName(i) + ": " + request->arg(i) + "\n";
  }

  request->send(404, "text/plain", message);
  digitalWrite(led, LOW);
}

void setup(void){
  pinMode(led, OUTPUT);
  pinMode(Thermistor_PIN, INPUT); // meat temp
  pinMode(Thermocouple_PIN, INPUT); // external temp
  Serial.begin(9600);

  if (!myservo.attach(ServoPin, Servo::CHANNEL_NOT_ATTACHED, 0, 180))
  {
    while(true);
  }
  myservo.write(0);

  digitalWrite(led, HIGH);
  delay(500);
  myservo.write(180);
  digitalWrite(led, LOW);
  delay(500);
  myservo.write(0);
  digitalWrite(led, HIGH);
  delay(500);
  

  myPID.SetMode(AUTOMATIC);

  WiFi.config(IPAddress(192,168,1,4)  /*ip*/, IPAddress(192,168,1,1 /*gw*/), IPAddress(255,255,255,0 /*sn mask*/));
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

#if 0
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
#endif //0

  WiFi.begin("Bearsden 2.0", "Peace10310313");
  Serial.println("\nConnecting");

  while(WiFi.status() != WL_CONNECTED){
      Serial.print(".");
      delay(100);
  }


  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  //Serial.println(wm.getWiFiSSID(false /*persistent*/));
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, handleRoot);

  server.on("/json", HTTP_GET, handleJson);

  server.on("/settemp", HTTP_POST, [](AsyncWebServerRequest * request){}, NULL, handleSetTemp);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTPS server started");

  digitalWrite(led, LOW);
}

float get_voltage(int raw_adc) {
  float raw = raw_adc;
  float aref = 3.3;
  float res = pow(2, ADC_RESOLUTION);
  
  return (raw * aref) / res;  
}

float get_temperature(float voltage) {
  return voltage * 200 - 250;
}

float readThermoCouple()
{
    int reading = 0;
    for (int i = 0; i < 10; i++)
    {
      delay(10);
      reading = reading + analogRead(Thermocouple_PIN);
    }

    reading = reading / 10;

    Serial.println("\nreading: " + String(reading));
    float voltage = get_voltage(reading);
    Serial.println("\nvoltage: " + String(voltage));
    return get_temperature(voltage);
}

float readThermistor()
{
  // read the pin value 10 times over 100 ms span and take average
  double sensorValue = 0;
  double vrefSensorValue = 0;
  const int dcount = 20;
  for (int i = 0; i < dcount; i++)
  {
    delay(10);
    sensorValue += analogRead(Thermistor_PIN);
  }

  sensorValue = sensorValue / dcount;

  Serial.println("\nTermistorSensor: " + String(sensorValue));

  double sensorMax = pow(2, ADC_RESOLUTION);
  // Convert the analog reading (which goes from 0 - 4096) to voltage reference (3.3V):
  double voltage = sensorValue * 3.3  / sensorMax;

  // i'm adjusting the misterious temperature error (for now treating it as constant error)
  // voltage = voltage - 0.028;

  double R_1 = R_2 * (V_0 - voltage)/voltage; // voltage to resistance

  double rThermistor = R_1;

  Serial.println("\nTermistorOhm: " + String(rThermistor));
  if (rThermistor < 0)
    return 0;

  double lnR = log(rThermistor);

  // T(K) = 1/(C0 + C1Ln(R) + C2(Ln(R))^3)
  double temp  = factor /(CM0 + lnR * CM1 + CM2 * lnR * lnR * lnR);
  temp  = temp - 273.15; // K -> C

  if (temp > 500)
    temp = 0;

  return temp;
}

void loop(void){
  int millisNow = millis();
  if (millisTimeLast == 0 || millisNow - millisTimeLast > (1000 * 10))
  {
    millisTimeLast = millisNow;
    int tempC = 0;
    tempC  = readThermoCouple();
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