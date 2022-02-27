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

static const uint8_t blackData[] =
  { B00000000,
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000,         
    B00000000 };
    
    
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

void showBitmap(int intensity, const uint8_t data[])
{
  if (intensity == 0)
  {
    setBitmap(blackData);
    mled.display();
    return;
  }
  
  mled.intensity = intensity - 1;
  setBitmap(data);
  mled.display();
}

void loop2() {
  setBitmap(smileData);
  delay(1000);
  setBitmap(rainData);
  delay(1000);
  setBitmap(sunData);
  delay(1000);
}

int i = 0;
void loop3() {

  while (i < 8)
  {
    showBitmap(i++, smileData);
    delay(500);
  }

  while (i > 0)
  {
    showBitmap(i - 1, smileData);
    i--;
    delay(500);
  }
}

void loop() {

  uint8_t tempData[8]={};

  for (int i = 7; i >= 0; i--)
  {
    for (int j = 0; j < 8; j++)
    {
      tempData[j] = smileData[(i + j) % 8];  
      tempData[j] = smileData[j];  
    }

    for (int j = 0; j < 8; j++)
    {
      uint8_t t = tempData[j];
      tempData[j] = (t << i) + (t >> (8 - i));
    }
    showBitmap(2, tempData);
    delay(100);

    if (i==0)
        delay(500);

  }
}
