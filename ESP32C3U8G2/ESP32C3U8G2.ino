//#define WIRE_INTERFACES_COUNT 2
//#define SOC_I2C_SUPPORTED true
//#define SOC_I2C_NUM 2
#include <Wire.h>
#include <U8g2lib.h> //C:\Users\kentu\Documents\Arduino\libraries\U8g2\src\U8g2lib.h

// there is no 72x50 constructor in u8g2 hence the 72x40 screen is mapped in the middle of the 132x64
// pixel buffer of SSD1306 controller

U8G2_SH1106_72X40_WISE_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
//U8G2_SH1106_72X40_WISE_F_2ND_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
int width = 72;
int height = 40;

//int xOffset = 30; //= 132-w/2
//int yOffset = 12; //=(64 -h)/2
int xOffset = 0; //= 132-w/2
int yOffset = 0; //=(64 -h)/2

byte addressFound = 0;

void i2cScan() {
  byte error;

  Wire.setPins(5/*SDA*/, 6 /*SCL*/);
  Wire.begin();

  for(byte address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      addressFound = address;
      //return;
    }
  }

  Wire.end();
}

void setup() {
  delay(1000);
  i2cScan();
  delay(1000);
  u8g2.setI2CAddress(0x3c);
  u8g2.begin();
  u8g2.setContrast(255);

  u8g2.setBusClock(400000); //400kHz I2C
  u8g2.setFont(u8g2_font_logisoso26_tf);
}



void loop() {
  u8g2.clearBuffer();
  u8g2.drawFrame(xOffset+0, yOffset+0, width, height);
  u8g2.setCursor(xOffset, yOffset+33);
  int addr = u8g2_GetI2CAddress(&u8g2);
  u8g2.printf(" %d", addressFound);
  u8g2.setCursor(xOffset+49, yOffset+33);
  u8g2.printf(".");
  u8g2.setCursor(xOffset+55, yOffset+33);
  u8g2.printf("0");
  u8g2.sendBuffer();
}
