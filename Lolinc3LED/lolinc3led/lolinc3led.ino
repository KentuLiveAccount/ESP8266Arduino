#include <Adafruit_NeoPixel.h>
#define PIN        7
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB);

#define DELAYVAL 10
const unsigned int colorDepth = 4;
const unsigned int maxNum = pow(2, colorDepth * 3);
const unsigned short maxColorVal = pow(2, colorDepth) - 1;

unsigned int num = 0;

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

void loop() {
  setColor(RFromNum(num), GFromNum(num), BFromNum(num));
  delay(DELAYVAL);

  num = (num + 1) % maxNum;
}
