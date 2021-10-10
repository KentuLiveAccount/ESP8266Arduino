#include <WEMOS_Matrix_LED.h>

const byte D7 = 13;
const byte D5 = 14;

MLED mled(0, D7, D5); //set intensity=0


void setup() {
  // put your setup code here, to run once:
 
}

static const uint8_t smileData[] =
  { B00111100,         
    B01111110,
    B11011011,
    B11111111,
    B11111111,
    B10111101,
    B01000010,
    B00111100 };

static const uint8_t sunData[] = {
    B00100100,
    B10111101,
    B01111110,         
    B01111110,
    B01111110,
    B01111110,
    B10111101,
    B00100100
};

static const uint8_t rainData[] =
  { B00001000,
    B00111110,         
    B01111111,
    B01111111,
    B00001000,
    B00001000,
    B00101000,
    B00010000 };

void allDots()
{
  for(int y=0;y<8;y++)
  {
    for(int x=0;x<8;x++)
    {
        mled.dot(x,y); // draw dot
        mled.display();
        delay(200);
        mled.dot(x,y,0);//clear dot
        mled.display();
        delay(200);        
    }  
  }
}

void setBitmap(const uint8_t data[])
{
  for (size_t i = 0; i < 8; i++)
  {
    mled.disBuffer[i] = data[i];
  }
  mled.display();
}

void loop() {
  setBitmap(smileData);
  delay(1000);
  setBitmap(rainData);
  delay(1000);
  setBitmap(sunData);
  delay(1000);
}
