#include <Arduino.h>
#include <WiFi.h> 
#include <I2S.h>

void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(1);

  Serial.begin(115200); //115200
  I2S.setAllPins(1 /*D1 - SCK*/, 2 /*D2 - FS*/, 3 /*D3 - SDOUT*/, 5 /*D3 - SDOut*/, 3 /*D4 SDIN*/);
  I2S.setSimplex();
  I2S.setBufferSize(1024); // 8 - 1024

  if (I2S.isDuplex())
    Serial.println("duplex mode");
  else
    Serial.println("simplex mode");

  if (I2S.begin(I2S_PHILIPS_MODE, 16000, 16))
    Serial.println("I2S begin success");
  else
    Serial.println("I2S begin failed");

  I2S.read(); // Switch the driver in simplex mode to receive
}

//const uint32_t dataMax = 65536 * 1;
const uint32_t dataMax = 1024 * 1;
int16_t data[dataMax];

bool dumpData = false;
byte *pb = (byte *)data;
const uint32_t cbMax = dataMax * sizeof(int16_t);
uint32_t cb = cbMax;

const uint8_t maxPeaks = 255;
uint32_t rgPeakIndex[maxPeaks];
int16_t rgPeakValue[maxPeaks];
uint8_t iPeak = 0;

void findPeaks()
{
  bool fStart = false;
  int16_t prevVal = 0;
  iPeak = 0;
  for (uint32_t i = 0; i < dataMax; i += 2)
  {
    if (prevVal > data[i]) // declining
    {
      //Serial.printf("declining\n");
      if ((iPeak < maxPeaks) && data[i] > 0 && fStart)
      {
        //Serial.printf("peak added\n");
        rgPeakIndex[iPeak] = i - 1;
        rgPeakValue[iPeak] = prevVal;
        iPeak++;
      }
      fStart = false;
    }
    else if (prevVal < data[i]) // rising
    {
      //Serial.printf("rising\n");
      fStart = true;
    }

    prevVal = data[i];
  }
}

void dumpPeaks()
{
  findPeaks();

  if (iPeak < 1)
  {
    Serial.printf("not enough peaks\n");
    return;
  }
  for (uint8_t i = 1; i < iPeak; i++)
  {
    Serial.printf("%d, %d\n", rgPeakIndex[i] - rgPeakIndex[i - 1], rgPeakValue[i - 1]);
  }
}

void dumpDataToSerial()
{
  for (uint32_t i = 0; i < dataMax; i += 2)
  {
    Serial.printf("%d,", data[i]);
  }
   Serial.printf("\n");
}

void loop() {
  uint32_t cbFilled = I2S.read((void *)pb, cb);

  if (cb <= cbFilled)
  {
    pb = (byte *)data;
    cb = cbMax;
    dumpData = true;
  }
  else
  {
    cb -= cbFilled;
    pb += cbFilled;
  }

  if (dumpData)
  {
    dumpData = false;
    dumpDataToSerial();
    dumpPeaks();
    Serial.println("dumpdata");
  }
  else
  {
    Serial.printf("added data: %d bytes\n", cbFilled);
  }
}
