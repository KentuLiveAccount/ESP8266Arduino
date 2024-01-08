#include <Arduino.h>
#include <WiFi.h> 
#include <I2S.h>
#include <arduinoFFT.h>

arduinoFFT FFT;

const uint16_t samples = 64;
const double samplingFrequency = 16000;

double vReal[samples];
double vImag[samples];

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03


void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(1);

  Serial.begin(115200); //115200
  I2S.setAllPins(1 /*D1 - SCK*/, 2 /*D2 - FS*/, 3 /*D3 - SDOUT*/, 5 /*D3 - SDOut*/, 3 /*D4 SDIN*/);
  I2S.setSimplex();
  I2S.setBufferSize(1024);

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

const uint32_t dataMax = 65536 * 1;
int16_t data[dataMax];

bool dumpData = false;
byte *pb = (byte *)data;
const uint32_t cbMax = dataMax * sizeof(int16_t);
uint32_t cb = cbMax;

void loop()
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

      for (uint16_t i=0; i < samples; i++)
      {
        // read every 4 data (l and r alternates but only l has data... 
        // the other 2 i don't know)
        vReal[i] = data[i * 4];
        vImag[i] = 0.0;
      }

      FFT = arduinoFFT(vReal, vImag, samples, samplingFrequency);

      Serial.println("Data:");
      PrintVector(vReal, samples, SCL_TIME);
      FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
      Serial.println("Weighed data:");
      PrintVector(vReal, samples, SCL_TIME);
      FFT.Compute(FFT_FORWARD);
      Serial.println("Computed Real values:");
      PrintVector(vReal, samples, SCL_INDEX);
      Serial.println("Computed Imaginary values:");
      PrintVector(vImag, samples, SCL_INDEX);
      FFT.ComplexToMagnitude();
      Serial.println("Computed magnitudes:");
      PrintVector(vReal, (samples >> 1), SCL_FREQUENCY);
      double x = FFT.MajorPeak();
      Serial.println(x, 6);

      delay(3000);
  }
}

void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
{
  for (uint16_t i = 0; i < bufferSize; i++)
  {
    double abscissa;

    switch (scaleType)
    {
      case SCL_INDEX:
        abscissa = i * 1.0;
        break;
      case SCL_TIME:
        abscissa = ((i *1.0)/samplingFrequency);
        break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * samplingFrequency)/samples);
        break;
    }

    Serial.print(abscissa, 6);
    if (scaleType == SCL_FREQUENCY)
      Serial.print("Hz");
    Serial.print(" ");
    Serial.println(vData[i], 4);
  }

  Serial.println();
}
