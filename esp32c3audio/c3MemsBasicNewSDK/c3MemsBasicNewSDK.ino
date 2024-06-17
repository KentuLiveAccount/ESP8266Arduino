#include <Arduino.h>
#include <WiFi.h> 
#include <ESP_I2S.h>

I2SClass I2S;

void setup() {
  WiFi.disconnect(true /*wifioff*/, false /*eraseap*/);
  delay(3000);

  Serial.begin(115200); //115200
  I2S.setPins(1 /*D1 - SCK*/, 2 /*D2 - FS*/, -1 /*D3 - SDOUT*/, 3 /*D3 - SDIn*/, -1 /*clk*/);
  if (I2S.begin(I2S_MODE_STD, 96000 /* (48khz * 2) max 600000 */, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO))
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
      //for (uint32_t j = 0; j < dataMax; j++)
      for (uint32_t j = 0; j < dataMax; j+=4)
        Serial.printf("%d\n", data[j]);
      dumpData = false;
  }
}
