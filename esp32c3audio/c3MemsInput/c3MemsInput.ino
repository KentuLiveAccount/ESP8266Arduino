#include <Arduino.h>
#include <WiFi.h> 
#include <I2S.h>


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
      // read every 4 data (l and r alternates but only l has data... 
      the other 2 i don't know)
      for (uint32_t j = 0; j < dataMax; j+=4)
        Serial.printf("%d\n", data[j]);
      dumpData = false;
  }

#if 0
  if (count > 0 && i < dataMax)
  {
    for (int32_t x = 0; x < count; x++) 
    {
//       if (abs(buffer[x]) < 50)
//       {
//          Serial.printf("%d\n", 0);
//          continue;
//       }
       if (i < dataMax)
          data[i++] = buffer[x];

//       Serial.printf("%d\n", buffer[x]);
    }
  }
  else if (i >= dataMax)
  {
    for (uint32_t j = 0; j < dataMax; j++)
      Serial.printf("%d\n", data[j]);

    i = 0;
  }

  //Serial.printf("%d, %d\n", l, r);
#endif //0
}
