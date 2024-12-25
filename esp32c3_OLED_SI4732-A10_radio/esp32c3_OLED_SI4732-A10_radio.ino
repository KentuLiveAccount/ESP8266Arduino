/*
   Test and validation of the SI4735 Arduino Library and ESP8266.

   ATTENTION:  Please, avoid using the computer connected to the mains during testing.

   The main advantages of using this sketch are: 
    1) It is an easy way to check if your circuit is working
    2) You do not need to connect any display device to make your radio works
    3) You do not need connect any push buttons or encoders to change volume and frequency
    4) The Arduino IDE is all you need to control the radio
   
   This sketch has been successfully tested on ESP8266 on SI4735-G60 or SI4732-A10

  | Si4735-G60 | Function              |   ESP8266             |
  |------------| ----------------------|-----------------------|
  | pin 15     |   RESET               |   2 (GPIO2)           |  
  | pin 18     |   SDIO                |   4 (SDA / GPIO4)     |
  | pin 17     |   SCLK                |   5 (SCL / GPIO5)     |


  | Si4732-A10 | Function              |   ESP8266             |
  |------------| ----------------------|-----------------------|
  | pin 9      |   RESET               |   2 (GPIO2)           |  
  | pin 12     |   SDIO                |   4 (SDA / GPIO4)     |
  | pin 11     |   SCLK                |   5 (SCL / GPIO5)     |



  Prototype documentation : https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

  By Ricardo Lima Caratti, May 2021.
*/

#define WIRE_INTERFACES_COUNT 2

#include <SI4735.h>
//#include <BLEDevice.h>
#include <ezButton.h>  
#include <WiFi.h>
#include <U8g2lib.h> //C:\Users\kentu\Documents\Arduino\libraries\U8g2\src\U8g2lib.h

// rotary encoder
#define CLK_PIN 1 //GPIO1
#define DT_PIN 2  // GPIO2
#define SW_PIN 0  // GPIO0

#define DIRECTION_CW 0   // clockwise direction
#define DIRECTION_CCW 1  // counter-clockwise direction

int CLK_state;
int prev_CLK_state;

ezButton button(SW_PIN);  // create ezButton object that attach to pin 4

// OLED SH1106

//U8G2_SH1106_72X40_WISE_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
//U8G2_SH1106_72X40_WISE_F_2ND_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
int width = 72;
int height = 40;

// SI 4732
#define RESET_PIN 10           // (GPIO10/10)

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 8       // (GPIO8/8)
#define ESP32_I2C_SCL 9       // (GPIO09/9)

#define AM_FUNCTION 1
#define FM_FUNCTION 0

uint16_t currentFrequency;
uint16_t previousFrequency;
uint8_t bandwidthIdx = 0;
const char *bandwidth[] = {"6", "4", "3", "2", "1", "1.8", "2.5"};
enum band {
  fm = 0,
  am = 1,
  lw = 2,
  line = 3,
  band_count = 4
};

uint8_t curBand = fm;

SI4735 si4735;

void showHelp()
{
  Serial.println("Type F to FM; A to MW; L to LW; and 1 to SW");
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type B to change Bandwidth filter");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(1000);
}

// Show current frequency
void showStatus()
{
  si4735.getStatus();
  si4735.getCurrentReceivedSignalQuality();
  Serial.print("You are tuned on ");
  if (si4735.isCurrentTuneFM())
  {
    Serial.print(String(currentFrequency / 100.0, 2));
    Serial.print("MHz ");
    Serial.print((si4735.getCurrentPilot()) ? "STEREO" : "MONO");
  }
  else
  {
    Serial.print(currentFrequency);
    Serial.print("kHz");
  }
  Serial.print(" [SNR:");
  Serial.print(si4735.getCurrentSNR());
  Serial.print("dB");

  Serial.print(" Signal:");
  Serial.print(si4735.getCurrentRSSI());
  Serial.println("dBuV]");
  Serial.print(" Volume:");
  Serial.print(si4735.getCurrentVolume());
  Serial.println("[0-63]");
  Serial.flush();
}

void outputFrequencyToOLED()
{
  U8G2_SH1106_72X40_WISE_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, 6 /*SCL*/, 5/*SDA*/);
  u8g2.begin();
  u8g2.setContrast(255);

  u8g2.setBusClock(400000); //400kHz I2C
  u8g2.setFont(u8g2_font_logisoso26_tf);

  u8g2.clearBuffer();
  u8g2.drawFrame(0, 0, width, height);
  u8g2.setCursor(0, 33);

  if (curBand == line)
  {
    u8g2.printf("LINE");
  }
  else if (curBand == fm)
  {
    int aboveDecimal = currentFrequency / 100; 

    if (aboveDecimal < 100)
      u8g2.printf(" %d", aboveDecimal);
    else
      u8g2.printf("%d", aboveDecimal);
      
    u8g2.setCursor(49, 33);
    u8g2.printf(".");
    u8g2.setCursor(55, 33);
    int belowDecimal = (currentFrequency - aboveDecimal * 100) / 10;
    u8g2.printf("%d", belowDecimal);
  }
  else if (currentFrequency < 1000)
  {
    u8g2.printf(" %d", currentFrequency);
  }
  else
  {
    u8g2.printf("%d", currentFrequency);
  }

  u8g2.sendBuffer();

  Wire.end();
}
void setupSI4732()
{
  digitalWrite(RESET_PIN, HIGH);
  Serial.println("AM and FM station tuning test.");

  showHelp();
  
  // The line below may be necessary to setup I2C pins on ESP32
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

  // Gets the current SI473X I2C address 
  int si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);
  si4735.setup(RESET_PIN, FM_FUNCTION);
  delay(300);

  // Starts default radio function and band (FM; from 84 to 108 MHz; 103.9 MHz; step 100kHz)
  si4735.setFM(8400 /* freqMin */, 10800 /* freqMax */, 9490 /*initialFreq*/, 10 /* freqStep */);
  si4735.setFmStereoOn();
  currentFrequency = previousFrequency = si4735.getFrequency();
  si4735.setVolume(63);

  showStatus();

  Wire.end();
}

void setupRotaryEncoder()
{
  // configure encoder pins as inputs
  pinMode(CLK_PIN, INPUT_PULLUP);
  pinMode(DT_PIN, INPUT_PULLUP);
  pinMode(SW_PIN, INPUT_PULLUP);
  button.setDebounceTime(50);  // set debounce time to 50 milliseconds
  // read the initial state of the rotary encoder's CLK pin
  CLK_state = prev_CLK_state = digitalRead(CLK_PIN);
}

void setup()
{
  Serial.begin(9600);
  while(!Serial);
    delay(1000);
  
  WiFi.mode( WIFI_MODE_NULL );

  setupRotaryEncoder();

  setupSI4732();

  outputFrequencyToOLED();
}

void handleSerialInput()
{
  if (Serial.available() <= 0)
    return;

  char key = Serial.read();
  switch (key)
  {
  case '+':
    si4735.volumeUp();
    break;
  case '-':
    si4735.volumeDown();
    break;
  case 'a':
  case 'A':
    si4735.setVolume(63);
    si4735.setAM(570, 1710, 810, 10);
    break;
  case 'f':
  case 'F':
    si4735.setVolume(63);
    si4735.setFM(8600, 10800, 9490, 10);
    si4735.setFmStereoOn();

    break;
  case '1':
    si4735.setVolume(63);
    si4735.setAM(9400, 9990, 9600, 5);
    break;
  case 'U':
  case 'u':
    si4735.frequencyUp();
    break;
  case 'D':
  case 'd':
    si4735.frequencyDown();
    break;
  case 'b':
  case 'B':
    if (si4735.isCurrentTuneFM())
    {
      Serial.println("Not valid for FM");
    }
    else
    {
      if (bandwidthIdx > 6)
        bandwidthIdx = 0;
      si4735.setBandwidth(bandwidthIdx, 1);
      //Serial.print("Filter - Bandwidth: ");
      //Serial.print(String(bandwidth[bandwidthIdx]));
      //Serial.println(" kHz");
      bandwidthIdx++;
    }
    break;
  case 'S':
    si4735.seekStationUp();
    break;
  case 's':
    si4735.seekStationDown();
    break;
  case '0':
    showStatus();
    break;
  case '?':
    showHelp();
    break;
  default:
    break;
  }
}

int deltaCount = 0;
const unsigned int c_FreqUpdateInterval = 250;
unsigned int millisFrequencyUpdateStart;

// Main
void loop()
{
  button.loop();  // MUST call the loop() function first

  // read the current state of the rotary encoder's CLK pin
  CLK_state = digitalRead(CLK_PIN);

  // If the state of CLK is changed, then pulse occurred
  // React to only the rising edge (from LOW to HIGH) to avoid double count
  if (CLK_state != prev_CLK_state && CLK_state == HIGH)
  {
    if (millisFrequencyUpdateStart == 0)
      millisFrequencyUpdateStart = millis();

    // if the DT state is HIGH
    // the encoder is rotating in counter-clockwise direction => decrease the counter
    if(digitalRead(DT_PIN) == HIGH)
    {
      // direction= DIRECTION_CCW;
      deltaCount -= 1;
    }
    else
    {
      // the encoder is rotating in clockwise direction => increase the counter
      // direction= DIRECTION_CW;
      deltaCount += 1;
    }
  }

  // save last CLK state
  prev_CLK_state = CLK_state;

  if(button.isPressed())
  {
    Serial.println("The button is pressed");
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

    curBand = (curBand + 1) % band_count;
    switch (curBand)
    {
    case am:
      si4735.setVolume(63);
      si4735.setAM(570, 1710, 810, 10);
      break;
    case fm:
      si4735.setVolume(63);
      si4735.setFM(8600, 10800, 9490, 10);
      si4735.setFmStereoOn();
      break;
    case lw:
      si4735.setVolume(63);
      si4735.setAM(9400, 9990, 9600, 5);
      break;
    case line:
      si4735.setVolume(0);
      break;
    }

    if (curBand == line)
      currentFrequency = 0;
    else
      currentFrequency = si4735.getCurrentFrequency();

    showStatus();
    Wire.end();
  }
  else if (millisFrequencyUpdateStart > 0)
  {
    if (millisFrequencyUpdateStart + c_FreqUpdateInterval < millis())
    {
      millisFrequencyUpdateStart = 0;

      if (deltaCount != 0)
      {
        currentFrequency += deltaCount * 10;
        deltaCount = 0;

        Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
        si4735.setFrequency(currentFrequency);
        currentFrequency = si4735.getCurrentFrequency();
        showStatus();
        Wire.end();
      }
    }
  }
  else if (Serial.available() > 0)
  {
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    handleSerialInput();
    currentFrequency = si4735.getCurrentFrequency();
    showStatus();
    Wire.end();
  }

  if (currentFrequency != previousFrequency)
  {
    previousFrequency = currentFrequency;
    outputFrequencyToOLED();
  }


  delay(1);
}