
const int outputInterval = 50;
int analogValue = 0;

int millisNext = 0;

void setup() {
  Serial.begin(115200);
  //pinMode(?)
  millisNext = millis() + outputInterval;

}


void loop() {

  int millisNow = millis();

  //analogValue = max(analogValue, analogRead(A0));
  analogValue = analogRead(A0);

  if (millisNow >= outputInterval)
  {
    millisNext = millis() + outputInterval;
    Serial.println(analogValue);
    analogValue = 0;
  }
  delay(5);
}
