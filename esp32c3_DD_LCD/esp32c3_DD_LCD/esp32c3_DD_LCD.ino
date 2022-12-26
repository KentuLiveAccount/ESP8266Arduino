const int com0 = 5; // GPIO5
const int com1 = 2; // GPIO2
const int com2 = 1; // GPIO1
const int com3 = 0; // GPIO0
const int pin2 = 6; // GPIO6

unsigned long millisLast = 0;
bool fOn = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(com0, OUTPUT);
  pinMode(com1, OUTPUT);
  pinMode(com2, OUTPUT);
  pinMode(com3, OUTPUT);
  pinMode(pin2, OUTPUT);
  digitalWrite(com0, 0);
  digitalWrite(com1, 0);
  digitalWrite(com2, 0);
  digitalWrite(com3, 0);
  digitalWrite(pin2, 0);
}

void loop() {

  unsigned long millisNow = millis();

  if (millisLast + 5 < millisNow)
  {
    digitalWrite(com0, fOn ? 1 : 0);
    digitalWrite(com1, fOn ? 1 : 0);
    digitalWrite(com2, fOn ? 1 : 0);
    digitalWrite(com3, fOn ? 1 : 0);
    digitalWrite(pin2, !fOn ? 1 : 0);

    fOn = !fOn;
    millisLast = millisNow;  
  }
}
