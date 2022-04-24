#include <SPI.h>

#define CS_3202 15

int channel0;
int channel1;
long channel0accumulator;
long channel1accumulator;
int numberofsamples;
double channel0avg;
double channel1avg;

int numberofseconds = 1;

const double ADCsteps = 4096.00;

// Current calibration constants
const double supplyvoltage = 3.3; //voltage dictating Vref for ADC

unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 1000;           // interval at which to blink (milliseconds)

void setup()
{
  Serial.begin(9600);
  pinMode(CS_3202, OUTPUT);
  digitalWrite(CS_3202, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);

}

void loop()
{
  channel0 = (Read3202(0));
  channel1 = (Read3202(1));

  //SPI.end();

#if 0
  Serial.print(channel0);
  Serial.print(",");
  Serial.print(channel1);
  Serial.print("\n");
#else
  if (numberofsamples < 200)
  {
    channel0accumulator = channel0accumulator + channel0;
    channel1accumulator = channel1accumulator + channel1;
    numberofsamples++;
  }
  /*  Serial.print("No. - ");
    Serial.println(numberofsamples);
    Serial.println(channel0accumulator);
    Serial.println(channel1accumulator);
    delay(100);
  */
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    Serial.println("START");
    Serial.println(numberofsamples);

    channel0avg = (double)channel0accumulator / (double)numberofsamples;
    channel1avg = (double)channel1accumulator / (double)numberofsamples;

    double ratioC0 = channel0avg / 4096.00;
    double sampledVoltageC0 = ratioC0 * supplyvoltage;

    double ratioC1 = channel1avg / 4096.00;
    double sampledVoltageC1 = ratioC1 * supplyvoltage;

    Serial.print("channel0avg - ");
    Serial.println(channel0avg);
    Serial.print("sampledVoltageC0 - ");
    Serial.println(sampledVoltageC0 * 100.0);

    Serial.print("channel1avg - ");
    Serial.println(channel1avg);
    Serial.print("sampledVoltageC1 - ");
    Serial.println(sampledVoltageC1 * 100.0);

    Serial.print("numberofseconds - ");
    Serial.println(numberofseconds);
    Serial.println();
    numberofseconds++;

    numberofsamples = 0;
    channel0accumulator = 0;
    channel1accumulator = 0;
  }
  #endif
}

int Read3202(int CHANNEL) {
  int msb;
  int lsb;
  int commandBytes = B10100000 ;// channel 0
  if (CHANNEL == 1) 
    commandBytes = B11100000 ; // channel 1

  digitalWrite(CS_3202, LOW);
  delay(2);

  SPI.transfer (B00000001);// Start bit
  msb = SPI.transfer(commandBytes) ;
  msb = msb & B00001111;
  lsb = SPI.transfer(0x00);
  digitalWrite(CS_3202, HIGH);

  return ((int) msb) << 8 | lsb;
}
