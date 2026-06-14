/*
   Test and validation of the SI4735 Arduino Library and ESP32.

   ATTENTION:  Please, avoid using the computer connected to the mains during testing.

   The main advantages of using this sketch are: 
    1) It is a easy way to check if your circuit is working;
    2) You do not need to connect any display device to make your radio works;
    3) You do not need connect any push buttons or encoders to change volume and frequency;
    4) The Arduino IDE is all you need to control the radio.  
   
   This sketch has been successfully tested on:
    1) Pro Mini 3.3V; 
    2) UNO (by using a voltage converter); 
    3) Arduino Yún;
    4) Arduino Mega (by using a voltage converter); and 
    5) ESP32 (LOLIN32 WEMOS)

  | Si4735    | Function              |ESP LOLIN32 WEMOS (GPIO) |
  |-----------| ----------------------|-------------------------|
  | pin 15    |   RESET               |   12 (GPIO12)           |  
  | pin 18    |   SDIO                |   21 (SDA / GPIO21)     |
  | pin 17    |   SCLK                |   22 (SCL / GPIO22)     |


   I strongly recommend starting with this sketch.

   Prototype documentation : https://pu2clr.github.io/SI4735/
   PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

   By Ricardo Lima Caratti, Nov 2019.
*/

#include <SI4735.h>
#include "ESP_I2S.h"
#include "BluetoothA2DPSink.h"

const uint8_t I2S_SCK = 17;       /* Audio data bit clock */
const uint8_t I2S_WS = 22;       /* Audio data left and right clock */
const uint8_t I2S_SDOUT = 21;    /* ESP32 audio data output (to speakers) */
const uint8_t TS5VSWITCH = 16;   /* audio out switch low = rado, high bluetooth */
I2SClass i2s;

BluetoothA2DPSink a2dp_sink(i2s);

// Set from the A2DP callback (Bluedroid task context); consumed in loop().
volatile bool btConnected = false;
volatile bool btStateChanged = false;

void onA2DPConnectionStateChanged(esp_a2d_connection_state_t state, void *)
{
  if (state == ESP_A2D_CONNECTION_STATE_CONNECTED) {
    btConnected = true;
    btStateChanged = true;
  } else if (state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
    btConnected = false;
    btStateChanged = true;
  }
}

#define RESET_PIN 26

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 19
#define ESP32_I2C_SCL 18



#define AM_FUNCTION 1
#define FM_FUNCTION 0

uint16_t currentFrequency;
uint16_t previousFrequency;
uint8_t bandwidthIdx = 0;
const char *bandwidth[] = {"6", "4", "3", "2", "1", "1.8", "2.5"};

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
}

bool FMBT = true;

void setup()
{
  Serial.begin(115200);
  delay(500);
  //while(!Serial);

  pinMode(RESET_PIN, OUTPUT);
  pinMode(TS5VSWITCH, OUTPUT);
  digitalWrite(RESET_PIN, HIGH);
  digitalWrite(TS5VSWITCH, FMBT ? LOW : HIGH);
  
  i2s.setPins(I2S_SCK, I2S_WS, I2S_SDOUT);
  if (!i2s.begin(I2S_MODE_STD, 44100, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, I2S_STD_SLOT_BOTH)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }

  a2dp_sink.set_on_connection_state_changed(onA2DPConnectionStateChanged);
  a2dp_sink.start("MyMusic");

  Serial.println("AM and FM station tuning test.");

  showHelp();

  // The line below may be necessary to setup I2C pins on ESP32
  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);

  int si4735Addr = si4735.getDeviceI2CAddress(RESET_PIN);

  delay(500);
  //si4735.setup(RESET_PIN, FM_FUNCTION);
  si4735.setup(RESET_PIN, 0 /* ctsIntEnable */, FM_FUNCTION, SI473X_ANALOG_AUDIO, XOSCEN_RCLK,0 /* gpo2Enable */);
  delay(500);

  // Starts defaul radio function and band (FM; from 84 to 108 MHz; 103.9 MHz; step 100kHz)
  si4735.setFM(8400, 10800, 9490, 10);

  delay(500);
  currentFrequency = previousFrequency = si4735.getFrequency();
  si4735.setVolume(55);
  showStatus();
}


// Main
void loop()
{
  if (btStateChanged)
  {
    btStateChanged = false;
    FMBT = !btConnected;
    digitalWrite(TS5VSWITCH, FMBT ? LOW : HIGH);
    Serial.println(btConnected
                     ? "Bluetooth connected -> audio: Bluetooth"
                     : "Bluetooth disconnected -> audio: Radio");
  }

  if (Serial.available() > 0)
  {
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
      si4735.setAM(570, 1710, 810, 10);
      break;
    case 'f':
    case 'F':
      si4735.setFM(8600, 10800, 9490, 10);
      break;
    case '1':
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
        Serial.print("Filter - Bandwidth: ");
        Serial.print(String(bandwidth[bandwidthIdx]));
        Serial.println(" kHz");
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
    case 't':
    case 'T':
      FMBT = !FMBT;
      digitalWrite(TS5VSWITCH, FMBT ? LOW : HIGH);
      break;
    default:
      break;
    }
  }
  
  delay(100);
  currentFrequency = si4735.getFrequency();
  if (currentFrequency != previousFrequency)
  {
    previousFrequency = currentFrequency;
    showStatus();
    delay(300);
  }
  
}
