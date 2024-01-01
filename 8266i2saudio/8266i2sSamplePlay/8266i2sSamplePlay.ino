#include <Arduino.h> 
#include "ESP8266WiFi.h"
#include <I2S.h>
#include <i2s_reg.h>
#include <pgmspace.h>

#include "sampledata.h"
//const uint16_t BD16[144000] PROGMEM = {
//};

const uint32_t countMax = sizeof(BD16)/sizeof(uint16_t);

void setup() {
  WiFi.forceSleepBegin();
  delay(1);
  system_update_cpu_freq(160);
  i2s_begin();
  i2s_set_rate(44100);
}

uint32_t counter = 0;

void loop()
{
 int16_t DAC= pgm_read_word_near(BD16 + counter++);

 if (counter >= countMax)
  counter = 0;

 uint32_t lrv = DAC;
 lrv = (lrv & 0xffff) << 16 | (lrv) & 0xffff;
 i2s_write_sample(lrv);    
}
