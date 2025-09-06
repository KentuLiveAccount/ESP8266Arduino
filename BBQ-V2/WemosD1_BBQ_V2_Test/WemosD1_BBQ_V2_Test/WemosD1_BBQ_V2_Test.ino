/*
Wemos	Peripheral	
D2/GPIO4/SDA	  NTC Mux       1/A0	Digital Out
D0/GPIO16	      Resister Mux  1/A0	Digital Out
D3/GPIO0	      Resister Mux 10/A1	Digital Out
D1/GPIO5/SCL	  Servo         3	    PWM Out

D8/GPIO15	      MCP3202       1/CS/SHDN	
D5/GPIO14	      MCP3202       7/CLK	
D7/GPIO13/MOSI	MCP3202       5/DIN	
D6/GPIO12/MISO	MCP3202       6/DOUT	
*/
#include "MCP3202.h"
#include "ThermistorMath.h"
#include "RunningAverage.h"
// pins

#define led D4
#define NTCMuxSelect D2
#define ResMuxSelect0 D0
#define ResMuxSelect1 D3
#define MCP3202_CS D8

#define MCP_TestIn 0
#define MCP_TempIn 1

#define ServoPWM D1

// other constants
double V_0 = 3.295; // 3.3v is the standard supply

double ResistorBank0 = 9951.5592; //supposed to be 100k - not based on real measurement
double ResistorBank1 =  9951.5592; //supposed to be 10k.
double ResistorBank2 =  9951.5592; //supposed to be 100k.

#if 0

double ResistorBank0 = 100000.0; //supposed to be 100k - not based on real measurement
double ResistorBank1 =  10142.0; //supposed to be 10k.
double ResistorBank2 =  80620.0; //supposed to be 100k.

double ResistorBank0 = 86600.0; //supposed to be 100k - not based on real measurement
double ResistorBank1 =  9960.0; //supposed to be 10k.
double ResistorBank2 = 86700.0; //supposed to be 100k.
#endif //0

double rgRegistors[] = {ResistorBank0, ResistorBank1, ResistorBank2};

MCP3202 mcp(MCP3202_CS);

#define amb 0
#define meat 1

Thermistor::ThermistorValues rgThermistor[] =
{
#if 0
  {
    0, //channel zero is ambient
    10128.98342, //1.012898342 e-3 / 0.001012898342 * 10 ^ 7
    1693.975682, //1.693975682 e-4 / 0.0001693975682 * 10 ^ 7
    2.563810106  //2.563810106 e-7 / 0.0000002563810106 * 10 ^ 7
  },
#endif //0
  { // take 2
    0, //channel zero is ambient
    8471.838980, //0.8471838980 e-3 / 0.0008471838980 * 10 ^ 7
    1936.228867, //1.936228867 e-4 / 0.0001936228867 * 10 ^ 7
    1.866445809  //1.866445809 e-7 / 0.0000001866445809 * 10 ^ 7
  },
  {
    1, //channel one is meat
    10083.94444, //1.008394444 e-3 / 0.001008394444 * 10 ^ 7
    1699.832481, //1.699832481 e-4 / 0.0001699832481 * 10 ^ 7
    2.539205261 //2.539205261 e-7 / 0.0000002539205261 * 10 ^ 7
  }
};

void SelectResistorBank(int i)
{
  digitalWrite(ResMuxSelect0, (i % 2) == 0 ? LOW : HIGH);
  digitalWrite(ResMuxSelect1, ((i / 2) % 2) == 0 ? LOW : HIGH);
}

const int iRegistor0 = 0; //100k
const int iRegistorA = 1; //10k
const int iRegistorB = 2; //100k
const int iRegistor3 = 3; //N/C

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(led, OUTPUT);
  pinMode(NTCMuxSelect, OUTPUT);
  pinMode(ResMuxSelect0, OUTPUT);
  pinMode(ResMuxSelect1, OUTPUT);
  pinMode(ServoPWM, OUTPUT);
  pinMode(MCP3202_CS, OUTPUT);

  mcp.Initialize();


  digitalWrite(NTCMuxSelect, LOW);
  digitalWrite(ResMuxSelect0, LOW);
  digitalWrite(ResMuxSelect1, LOW);

  Serial.print("Hello\n");
  //digitalWrite(ResMuxSelect0, HIGH);
  //digitalWrite(ResMuxSelect1, HIGH);

  SelectResistorBank(iRegistor3); // NC

}

NestedRunningAverage<200, 50> avg0;
NestedRunningAverage<200, 50> avg1;


void loop1()
{

}

void loop2() 
{

  //SelectResistorBank(iRegistorA);

  delay(5);
  avg0.Add(mcp.Read(MCP_TempIn));

  //SelectResistorBank(iRegistorB);

  delay(5);
  avg1.Add(mcp.Read(MCP_TempIn));
  
  if (avg1.HasAverage())
  {
    float fRaw0 = avg0.CurValueInner();
    float favg0 = floor(avg0.AverageInner() + 0.5);
    float f0 =    avg0.Average();

    float fRaw1 = avg1.CurValueInner();
    float favg1 = floor(avg1.AverageInner() + 0.5);
    float f1 =    avg1.Average();

    float t10k = 0.0;
    float t100k = 4096.0;
    Serial.printf("%f, %f, %f, %f, %f, %f, %f, %f\n", fRaw0, fRaw1, favg0, favg1, f0, f1, t10k, t100k);
  }

  delay(5);
}


void loop() 
{
  int iRegistorA = 1; //10k
  int iRegistorB = 2; //100k

  SelectResistorBank(iRegistorA);

  delay(5);
  avg0.Add(mcp.Read(MCP_TempIn));

  SelectResistorBank(iRegistorB);

  delay(5);
  avg1.Add(mcp.Read(MCP_TempIn));
  
  if (avg1.HasAverage())
  {
    float fRaw0 = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(      avg0.CurValueInner(),       V_0, rgRegistors[iRegistorA]), rgThermistor[amb]));
    float favg0 = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(floor(avg0.AverageInner() + 0.5), V_0, rgRegistors[iRegistorA]), rgThermistor[amb]));
    float f0 =    Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(      avg0.Average(),             V_0, rgRegistors[iRegistorA]), rgThermistor[amb]));

    float fRaw1 = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(      avg1.CurValueInner(),       V_0, rgRegistors[iRegistorB]), rgThermistor[amb]));
    float favg1 = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(floor(avg1.AverageInner() + 0.5), V_0, rgRegistors[iRegistorB]), rgThermistor[amb]));
    float f1 =    Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(mcp.ohmFromADC(      avg1.Average(),             V_0, rgRegistors[iRegistorB]), rgThermistor[amb]));

    float t10k = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(10000.0, rgThermistor[amb]));
    float t100k = Thermistor::farenheightFromCelsius(Thermistor::CelciusFromOhm(100000.0, rgThermistor[amb]));
    Serial.printf("%f, %f, %f, %f, %f, %f, %f, %f\n", fRaw0, fRaw1, favg0, favg1, f0, f1, t10k, t100k);
  }

  delay(5);
}
