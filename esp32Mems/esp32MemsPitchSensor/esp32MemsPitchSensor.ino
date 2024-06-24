#include <Arduino.h>
#include <WiFi.h> 
#include <ESP_I2S.h>

I2SClass I2S;

void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(1);

  Serial.begin(115200); //115200
  I2S.setPins(0 /*D1 - SCK*/, 27 /*D2 - FS*/, -1 /*D3 - SDOUT*/, 25 /*D3 - SDIn*/, -1 /*clk*/);
  if (I2S.begin(I2S_MODE_STD, 96000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO))
    Serial.println("I2S begin success");
  else
    Serial.println("I2S begin failed");
}

//const uint32_t dataMax = 65536 / 2; // /2 to make it fit into default 1.2MB app
const uint32_t dataMax = 65536 / 2;
int16_t data[dataMax];

bool dataReady = false;
byte *pb = (byte *)data;
const uint32_t cbMax = dataMax * sizeof(int16_t);
uint32_t cb = cbMax;


const uint8_t maxPeaks = 255;
uint32_t rgPeakIndex[maxPeaks];
int16_t rgPeakValue[maxPeaks];
uint8_t iPeak = 0;

float peakScore(int16_t* pData, uint32_t cData, uint32_t cIncrements, uint32_t interval)
{
  int16_t *pData2 = pData + interval * cIncrements;
  if (pData + cData <= pData2)
    return 0.0;

  int16_t *pDataMax = pData + (cData);
  uint8_t count = 0;
  float phiSum = 0;
  float psiSum = 0;
  while (pData2 < pDataMax)
  {
    float d1 = *pData;
    float d2 = *pData2;
    phiSum += d1 * d2;
    psiSum += abs(d1 - d2);
    pData += cIncrements;
    pData2 += cIncrements;
    count++;
  }

  float phi = phiSum/((float)count);
  float psi = psiSum/((float)count);
  
  return phi / (psi + 1.0);
}

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
        rgPeakIndex[iPeak] = i - 4;
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

void interpolate(float y0, float y1, float y2, float &xPeak, float &yPeak)
{
  if (abs(y0) < abs(y2))
  {
    if (abs(y1) < abs(y2))
    {
      xPeak = 1.0;
      yPeak = y2;
      return;
    }

    yPeak = (y2 - y0 + y1 + y1) / 2;
    xPeak = (y2 - y0)/(2 * (y1 - y0));
  }
  else if (abs(y0) > abs(y2))
  {
    if (abs(y0) > abs(y1))
    {
      xPeak = -1.0;
      yPeak = y0;
      return;
    }

    yPeak = (y0 - y2 + y1 + y1) / 2;
    xPeak = (y0 - y2)/(2 * (y2 - y1));
  }
}

float scores[256]={};

void interpolateInterval()
{
  Serial.printf("InterpolateInterval\n");

  float scoreLargest1 = 0.0, scoreLargest2 = 0.0;
  uint32_t intervalLargest1 = 0, intervalLargest2 = 0;

  for (uint32_t interval = 25; interval < 256; interval++)
  {
    float scoreCur = peakScore(data, dataMax, 2 /*cIncrements*/, interval);
//    float scoreCur = peakScore(data2, 256, 1 /*cIncrements*/, interval);

    scores[interval] = scoreCur;

    if (scoreCur > scoreLargest1)
    {
      scoreLargest2 = scoreLargest1;
      intervalLargest2 = intervalLargest1;

      scoreLargest1 = scoreCur;
      intervalLargest1 = interval;
    }

    //Serial.printf("%d, %f\n", interval, scoreCur);
  }

  float interpolateInterval1 = intervalLargest1;
  float interpolateInterval2 = intervalLargest2;
  
  if (intervalLargest1 > 25 && intervalLargest1 < 255)
  {
    float xPeak = 0.0, yPeak = 0.0;
    interpolate(scores[intervalLargest1 - 1], scores[intervalLargest1], scores[intervalLargest1 + 1], xPeak, yPeak);
    interpolateInterval1 += xPeak;
    scoreLargest1 = yPeak;
  }

  if (intervalLargest2 > 25 && intervalLargest2 < 255)
  {
    float xPeak = 0.0, yPeak = 0.0;
    interpolate(scores[intervalLargest2 - 1], scores[intervalLargest2], scores[intervalLargest2 + 1], xPeak, yPeak);
    interpolateInterval2 += xPeak;
    scoreLargest2 = yPeak;
  }

  Serial.printf("#1, Largest %d, %f, %f Hz, %f, %f Hz\n", intervalLargest1, scoreLargest1, 48000.0 / ((float) intervalLargest1), interpolateInterval1, 48000.0 / interpolateInterval1);
  Serial.printf("#2, Largest %d, %f, %f Hz, %f, %f Hz\n", intervalLargest2, scoreLargest2, 48000.0 / ((float) intervalLargest2), interpolateInterval2, 48000.0 / interpolateInterval2);
}


void dumpPeakScores()
{
   Serial.printf("DumpPeakScores\n");

  float scoreLargest = 0.0;

  uint32_t intervalLargest = 0;

  for (uint32_t interval = 25; interval < 256; interval++)
  {
    float scoreCur = peakScore(data, dataMax, 2 /*cIncrements*/, interval);
//    float scoreCur = peakScore(data2, 256, 1 /*cIncrements*/, interval);

    if (scoreCur > scoreLargest)
    {
      scoreLargest = scoreCur;
      intervalLargest = interval;
    }

    Serial.printf("%d, %f\n", interval, scoreCur);
  }

  Serial.printf("Largest %d, %f, %f Hz\n", intervalLargest, scoreLargest, 48000.0 / ((float) intervalLargest));
}

void dumpPeaks()
{
  findPeaks();

  Serial.printf("DumpPeaks\n");

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

void dumpDataToSerialRaw()
{
  for (uint32_t i = 0; i < dataMax; i += 2)
  {
    Serial.printf("%d,", data[i]);
  }
   Serial.printf("\n");
}
void loop()
{

  uint32_t cbFilled = I2S.readBytes((char *)pb, cb);

  if (cb <= cbFilled)
  {
    pb = (byte *)data;
    cb = cbMax;
    dataReady = true;
  }
  else
  {
    cb -= cbFilled;
    pb += cbFilled;
  }

  if (dataReady)
  {
    //dumpDataToSerial();
    //dumpDataToSerialRaw();
    //dumpPeaks();
    dumpPeakScores();
    interpolateInterval();

    dataReady = false;
  }
}
