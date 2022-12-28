const int com1 = D1;
const int com2 = D2;
const int com3 = D3;
const int com4 = D4; 
const int pin1 = D0;
const int pin2 = D5;
const int pin3 = D6;
const int pin4 = D7;
const int pin5 = D8;


void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(com1, OUTPUT);
  pinMode(com2, OUTPUT);
  pinMode(com3, OUTPUT);
  pinMode(com3, OUTPUT);
  pinMode(com4, OUTPUT);
  pinMode(pin1, OUTPUT);
  pinMode(pin2, OUTPUT);
  pinMode(pin3, OUTPUT);
  pinMode(pin4, OUTPUT);
  pinMode(pin5, OUTPUT);

  digitalWrite(com1, 0);
  digitalWrite(com2, 0);
  digitalWrite(com3, 0);
  digitalWrite(com4, 0);

  digitalWrite(pin1, 0);
  digitalWrite(pin2, 0);
  digitalWrite(pin3, 0);
  digitalWrite(pin4, 0);
  digitalWrite(pin5, 0);
}

unsigned long millisLast = 0;
unsigned long millisLastCycle = 0;
bool fOn = false;
unsigned long iCycle = 0;

// cycles = 1c,           1b,           2a,           3a,           3b,           3c,           3d,           2d
//        = (pin5, com2), (pin5, com3), (pin3, com4), (pin1, com4), (pin1, com3), (pin1, com2), (pin2, com1), (pin4, com1)
unsigned long pinsCycle[]        = {pin5, pin5, pin3, pin1, pin1, pin1, pin2, pin4};
unsigned long comsCycle[]        = {com2, com3, com4, com4, com3, com2, com1, com1};
unsigned long coms[] = {com1, com2, com3, com4};

void loop() {

  unsigned long millisNow = millis();

  unsigned long comCur = comsCycle[iCycle];
  unsigned long pinCur = pinsCycle[iCycle];

  if (millisLastCycle + 1000 < millisNow)
  {
    iCycle = (iCycle + 1) % (sizeof(comsCycle)/sizeof(comsCycle[0]));
    millisLastCycle = millisNow;
  }

  if (millisLast + 5 < millisNow)
  {
    for (unsigned long i = 0; i < (sizeof(coms)/sizeof(unsigned long));i++)
    {
      if (comCur != coms[i])
        digitalWrite(coms[i], 0);
    }

    digitalWrite(comCur, fOn ? 1 : 0);
    digitalWrite(pinCur, !fOn ? 1 : 0);

    fOn = !fOn;
    millisLast = millisNow;  
  }
}
