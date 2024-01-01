#include <Arduino.h> 
#include "ESP8266WiFi.h"
#include <I2S.h>
#include <i2s_reg.h>
#include <pgmspace.h>

#include "sampledata.h"
// sampled data are signed 16bit values
//const uint16_t BD16[144000] PROGMEM = {
//};

const uint32_t countMax = sizeof(BD16)/sizeof(uint16_t);

void setup() {
  // turn off wifi
  WiFi.forceSleepBegin();
  delay(1);

  // set CPU to opearate at 160MHz
  system_update_cpu_freq(160);

  // 16bit per sample per channel
  i2s_set_bits(16);
  i2s_begin();
  // sampling requency == 44.1kHz
  i2s_set_rate(44100);
}

uint32_t counter = 0;

void loop()
{
 // use pgm_read_word_near to read off of the array in PROGMEM
 int16_t DAC= pgm_read_word_near(BD16 + counter++);

 if (counter >= countMax)
  counter = 0;

 uint32_t left = DAC;
 uint32_t right = DAC;

 //left half of the value is the sample value for left, right half is for right
 uint32_t lrv = ((left & 0xffff)) << 16 | (right) & 0xffff;
 i2s_write_sample(lrv);    
}
