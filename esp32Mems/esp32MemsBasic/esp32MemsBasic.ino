#include <Arduino.h>
#include <WiFi.h> 
#include <ESP_I2S.h>

I2SClass I2S;

void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(1);

  Serial.begin(115200); //115200
  I2S.setPins(0 /*D1 - SCK*/, 27 /*D2 - FS*/, -1 /*D3 - SDOUT*/, 25 /*D3 - SDIn*/, -1 /*clk*/);
  if (I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO))
    Serial.println("I2S begin success");
  else
    Serial.println("I2S begin failed");
}

const uint32_t dataMax = 65536 / 2;
int16_t data[dataMax];

bool dumpData = false;
byte *pb = (byte *)data;
const uint32_t cbMax = dataMax * sizeof(int16_t);
uint32_t cb = cbMax;

void loop()
{

  uint32_t cbFilled = I2S.readBytes((char *)pb, cb);

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
      // the other 2 i don't know)
      for (uint32_t j = 0; j < dataMax; j+=2)
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
