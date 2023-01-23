#include <WEMOS_Matrix_LED.h>
#include "wifisetting.h"
#include <ESP8266WiFi.h>
#include <WifiClient.h>
#include <time.h>

const char *TIME_SERVER ="pool.ntp.org";
const int myTimeZone = -8 * 60 * 60; //PST (PDT == )

const int EPOCH_1_1_2019 = 1546300800; // 01/01/2019 @ 12:00am (UTC)

time_t now;

//const byte D7 = 13;
//const byte D5 = 14;

MLED mled(0, D7, D5); //set intensity=0

static uint8_t matrixData[] = {};

#if 0
  { B00001000,
    B00111110,         
    B01111111,
    B01111111,
    B00001000,
    B00001000,
    B00101000,
    B00010000 };
#endif //0

enum {
  eSec = 0, // 0-59
  eMin = 1, //0-59
  eHour = 2, //0-23
  eDay = 3, //1-31 
  eMonth = 4, //1-12
  eYearLow = 5, //0-99
  eYearHigh = 6 //0-99
};

static uint8_t dates[7]={};

const uint8_t secMax = 60;
const uint8_t minMax = 60;
const uint8_t hrMax = 24;
const uint8_t dayMax = 32;
const uint8_t moMax = 13;

void addSec(uint8_t sec)
{
  uint8_t carryOver = (dates[eSec] + sec) / secMax; 
  dates[eSec] = (dates[eSec] + sec) % secMax;

  uint8_t carryOverM = (dates[eMin] + carryOver) / minMax;
  dates[eMin] = (dates[eMin] + carryOver) % minMax; 

  uint8_t carryOverH = (dates[eHour] + carryOverM) / hrMax;
  dates[eHour] = (dates[eHour] + carryOverM) % hrMax; 

  carryOver = (dates[eDay] + carryOverH) / dayMax;
  dates[eDay] = (dates[eDay] + carryOverH) % dayMax; 
}

void updateMatrixDataFromDates()
{
  uint8_t sec = dates[eSec];
  uint8_t secHigh = sec / 10;
  uint8_t secLow = sec - (secHigh * 10);
  matrixData[0] = secLow;
  matrixData[1] = secHigh;

  sec = dates[eMin];
  secHigh = sec / 10;
  secLow = sec - (secHigh * 10);
  matrixData[3] = secLow;
  matrixData[4] = secHigh;

  sec = dates[eHour];
  secHigh = sec / 10;
  secLow = sec - (secHigh * 10);
  matrixData[6] = secLow;
  matrixData[7] = secHigh;
}


void setBitmap(const uint8_t data[])
{
  for (size_t i = 0; i < 8; i++)
  {
    mled.disBuffer[i] = data[i];
  }
  mled.display();
}

void showBitmap(int intensity, const uint8_t data[])
{
  mled.intensity = intensity - 1;
  setBitmap(data);
  mled.display();
}

void setTimeFromNTC()
{
  struct tm *timeinfo;

  time(&now);
  timeinfo = localtime(&now);

  dates[eSec] = timeinfo->tm_sec;
  dates[eMin] = timeinfo->tm_min;
  dates[eHour] = timeinfo->tm_hour;
  dates[eDay] = timeinfo->tm_mday;
  dates[eMonth] = timeinfo->tm_mon;
  dates[eYearLow] = (timeinfo->tm_year + 1900) - 2000;
  dates[eYearHigh] = (timeinfo->tm_year + 1900) / 100;
  
}

void setup() {
  Serial.begin(115200);

  Serial.println("starting wifi");
  WiFi.begin(WIFINAME, WIFIPW);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("wifi started. connecting to ntp server");
  configTime(myTimeZone, 0, TIME_SERVER);

  while (now < EPOCH_1_1_2019)
  {
    now = time(nullptr);
    delay(500);
    Serial.print("*");
  }

  Serial.println("setting time");
  setTimeFromNTC();
}

int msecLast = 0;

void loop() {
  int msecCur = millis();

  if (msecLast + 1000 < msecCur)
  {
    msecLast = msecCur;
    addSec(1);
    updateMatrixDataFromDates();
    showBitmap(2, matrixData);  
  }
  // put your main code here, to run repeatedly:
  delay(100);
}
