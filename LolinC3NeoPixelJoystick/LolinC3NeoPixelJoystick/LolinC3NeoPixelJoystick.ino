#include <Adafruit_NeoPixel.h>
#define PIN        7 // NeoPixel LED
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB);

#define DELAYVAL 10
const unsigned int colorDepth = 4;
const unsigned int maxNum = pow(2, colorDepth * 3);
const unsigned short maxColorVal = pow(2, colorDepth) - 1;

unsigned int num = 0;

#define AnalogV A0  //Joystick Analog V
#define AnalogH A1  //Joystick Analog H

unsigned short RFromNum(unsigned int num)
{
  return (num >> (colorDepth * 2));
}

unsigned short GFromNum(unsigned int num)
{
  return (num >> (colorDepth)) & maxColorVal;
}

unsigned short BFromNum(unsigned int num)
{
  return num & maxColorVal;
}

void setup() {
  Serial.begin(115200);
  Serial.println("initialize NeoPixels library");

  pixels.begin();
}

void setColor(unsigned short r, unsigned short g, unsigned short b)
{
  #if 0
  Serial.print("SetColor: ");
  Serial.print(r);
  Serial.print(", ");
  Serial.print(g);
  Serial.print(", ");
  Serial.println(b);
  Serial.print("MaxNum: ");
  Serial.println(maxNum);
  #endif //0
  
  pixels.clear();
  pixels.setPixelColor(0, pixels.Color(r, g, b));
  pixels.show();
}

void showAnalogIn(int v, int h)
{

  Serial.print("Analaog In V: ");
  Serial.print(v);
  Serial.print(", H: ");
  Serial.println(h);
}

int hRangeToCol(int h)
{
  // h range: 875-2365-3800
  return 128 * max(0, (h - 875))/(4000 - 875);
}

int vRangeToCol(int h)
{
  // v range: 710-2375-4095
  return 128 * max(0, (h - 710))/(4095 - 710);
}

void loop() {
  int v = analogRead(AnalogV);
  int h = analogRead(AnalogH);


  showAnalogIn(v, h);

  setColor(vRangeToCol(v), 0, hRangeToCol(h));

  //setColor(RFromNum(num), GFromNum(num), BFromNum(num));
  delay(DELAYVAL);

  num = (num + 1) % maxNum;
}