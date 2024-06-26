#include <Arduino.h>
#include <WiFi.h> 
#include <I2S.h>


//const uint32_t dataMax = 65536 * 1;
const uint32_t dataMax = 1024 * 1;
int16_t data[dataMax * 2];

bool dumpData = false;
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
    phiSum += abs(d1 * d2);
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
  for (uint32_t i = 0; i < dataMax; i += 4)
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

float interpolate(float y0, float y1, float y2)
{
  if (y0 < y2)
  {
    float slope1 = y1 - y0;
    float c1 = y0;
    float slope2 = -1 * slope1;
    float c2 = y2 - 2 * slope2;

    float yPeak = (c1 + c2) / 2;

    float xPeak = ((yPeak - c1) / slope1) - 1.0;

    return xPeak;
  }
  else if (y0 > y2)
  {
    float slope2 = y2 - y1; //negative
    float c2 = y2;
    float slope1 = -1 * slope2;
    float c1 = y0 + 2 * slope2;

    float yPeak = (c1 + c2) / 2;

    float xPeak = ((yPeak - c1) / slope1) + 1.0;

    return xPeak;
  }

  return 0;
}

int16_t data2[]={252,243,199,136,55,-28,-109,-176,-228,-248,-244,-201,-140,-61,27,108,180,235,256,243,207,147,68,-12,-92,-168,-221,-245,-241,-204,-153,-77,16,104,176,228,251,256,231,164,87,8,-73,-145,-209,-240,-236,-205,-156,-88,7,92,164,224,256,263,228,175,107,28,-60,-137,-196,-233,-240,-224,-172,-100,-13,68,144,211,255,260,235,191,119,44,-45,-129,-192,-236,-248,-233,-188,-120,-41,47,131,196,243,256,236,196,128,44,-33,-116,-184,-228,-252,-233,-196,-133,-53,27,116,184,236,256,240,207,144,68,-20,-101,-165,-213,-244,-240,-204,-141,-65,20,107,179,239,263,256,216,168,91,0,-84,-156,-201,-237,-240,-208,-152,-72,7,92,172,227,259,256,228,179,103,16,-68,-140,-200,-236,-245,-216,-169,-96,-9,75,152,211,247,255,228,179,103,27,88,7,-73,-153,-209,-245,-252,-225,-181,-105,-20,63,144,203,247,248,220,171,103,23,-68,-144,-208,-245,-256,-240,-196,-128,-48,44,131,188,231,236,223,180,111,31,-53,-136,-197,-245,-265,-248,-205,-140,-61,27,112,179,231,244,228,192,135,51,-33,-116,-180,-228,-256,-245,-212,-144,-64,20,107,180,231,252,248,215,156,75,-12,-93,-161,-216,-245,-244,-209,-153,-76,11,96,171,228};
float scores[81]={};

void interpolateInterval()
{
  float scoreLargest = 0.0;
  uint32_t intervalLargest = 0;

  for (uint32_t interval = 10; interval <= 80; interval++)
  {
    float scoreCur = peakScore(data, dataMax * 2, 4 /*cIncrements*/, interval);
//    float scoreCur = peakScore(data2, 256, 1 /*cIncrements*/, interval);

    scores[interval] = scoreCur;

    if (scoreCur > scoreLargest)
    {
      scoreLargest = scoreCur;
      intervalLargest = interval;
    }

    //Serial.printf("%d, %f\n", interval, scoreCur);
  }

  float interpolateInterval = intervalLargest;
  
  if (intervalLargest > 10 && intervalLargest < 79)
    interpolateInterval += interpolate(scores[intervalLargest - 1], scores[intervalLargest], scores[intervalLargest + 1]);

  Serial.printf("Largest %d, %f, %f Hz, %f, %f Hz\n", intervalLargest, scoreLargest, 0.501346 * 16000.0 / ((float) intervalLargest), interpolateInterval, 0.501346 * 16000.0 / interpolateInterval);
}


void dumpPeakScores()
{
  float scoreLargest = 0.0;
  uint32_t intervalLargest = 0;

  for (uint32_t interval = 10; interval <= 80; interval++)
  {
//    float scoreCur = peakScore(data, dataMax * 2, 4 /*cIncrements*/, interval);
    float scoreCur = peakScore(data2, 256, 1 /*cIncrements*/, interval);

    if (scoreCur > scoreLargest)
    {
      scoreLargest = scoreCur;
      intervalLargest = interval;
    }

    Serial.printf("%d, %f\n", interval, scoreCur);
  }

  Serial.printf("Largest %d, %f, %f Hz\n", intervalLargest, scoreLargest, 0.501346 * 16000.0 / ((float) intervalLargest));
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
  for (uint32_t i = 0; i < dataMax; i += 4)
  {
    Serial.printf("%d,", data[i]);
  }
   Serial.printf("\n");
}

void dumpDataToSerialRaw()
{
  for (uint32_t i = 0; i < dataMax; i += 1)
  {
    Serial.printf("%d,", data[i]);
  }
   Serial.printf("\n");
}

void readProcess()
{
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
    //dumpDataToSerialRaw();
    //dumpPeaks();
    //dumpPeakScores();
    interpolateInterval();
    Serial.println("dumpdata");
  }
  else
  {
    Serial.printf("added data: %d bytes\n", cbFilled);
  }
}


void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(1000);

  Serial.begin(115200); //115200
  I2S.setAllPins(1 /*D1 - SCK*/, 2 /*D2 - FS*/, 3 /*D3 - SDOUT*/, 5 /*D3 - SDOut*/, 3 /*D4 SDIN*/);
  I2S.setSimplex();
  I2S.setBufferSize(1024); // set the maximum buffer size

  I2S.onReceive(readProcess);

  if (I2S.isDuplex())
    Serial.println("duplex mode");
  else
    Serial.println("simplex mode");

  if (I2S.begin(I2S_PHILIPS_MODE, 16000, 16)) // begin as Master mode
    Serial.println("I2S begin success");
  else
    Serial.println("I2S begin failed");

  I2S.read(); // Switch the driver in simplex mode to receive
}

void loop() {

}
