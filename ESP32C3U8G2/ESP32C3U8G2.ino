#include <U8g2lib.h> //C:\Users\kentu\Documents\Arduino\libraries\U8g2\src\U8g2lib.h
#include <Wire.h>

// there is no 72x50 constructor in u8g2 hence the 72x40 screen is mapped in the middle of the 132x64
// pixel buffer of SSD1306 controller

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
int width = 72;
int height = 40;

int xOffset = 30; //= 132-w/2
int yOffset = 12; //=(64 -h)/2


void setup() {
  delay(1000);
  u8g2.begin();
  u8g2.setContrast(255);

  u8g2.setBusClock(400000); //400kHz I2C
  u8g2.setFont(u8g2_font_ncenB10_tr);
}

void loop() {
  u8g2.clearBuffer();
  u8g2.drawFrame(xOffset+0, yOffset+0, width, height);
  u8g2.setCursor(xOffset+15, yOffset+25);
  u8g2.printf("%dx%d", width, height);
  u8g2.sendBuffer();
}
