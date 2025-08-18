/*
MCP3202_CS : data pin connected to MCP3202 1/CS pin
*/
#include <SPI.h>

class MCP3202
{
public:
MCP3202(int CS):m_cs(CS){}
const int m_cs;
const int m_adcResolution = 12;

void Initialize()
{
  digitalWrite(m_cs, HIGH);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV32);
}

int Read(int CHANNEL) {
  int msb;
  int lsb;
  int commandBytes = B10100000 ;// channel 0
  if (CHANNEL == 1) 
    commandBytes = B11100000 ; // channel 1

  digitalWrite(m_cs, LOW);
  delay(2);

  SPI.transfer (B00000001);// Start bit
  msb = SPI.transfer(commandBytes) ;
  msb = msb & B00001111;
  lsb = SPI.transfer(0x00);
  digitalWrite(m_cs, HIGH);

  return ((int) msb) << 8 | lsb;
}

double ohmFromADC(double adcIn, double v0, double r2)
{
  double sensorMax = pow(2, m_adcResolution);
  // Convert the analog reading (which goes from 0 - 4096) to voltage reference (3.3V):
  double voltage = adcIn * v0  / sensorMax;

  double R_1 = 0;

  if (voltage > 0)
    R_1 = r2 * (v0 - voltage)/voltage; // voltage to resistance

  return R_1;
}


};