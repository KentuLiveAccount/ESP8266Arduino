#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SparkFun_AS7331.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 1     // GPIO21
#define ESP32_I2C_SCL 0     // GPIO22 

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

SfeAS7331ArdI2C myUVSensor;

void setup() {
  Serial.begin(9600);

  Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);


  // Wait for display
  delay(500);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

    // Initialize sensor and run default setup.
    if (myUVSensor.begin() == false)
    {
        Serial.println("Sensor failed to begin. Please check your wiring!");
        Serial.println("Halting...");
        while (1)
            ;
    } 

    Serial.println("Sensor began.");

    if (myUVSensor.prepareMeasurement(MEAS_MODE_CMD) == false)
    {
        Serial.println("Sensor did not get set properly.");
        Serial.println("Halting...");
        while (1)
            ;
    }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

    // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
}

void showUV(void) {

  display.clearDisplay();

  display.setTextSize(1); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  if (ksfTkErrOk != myUVSensor.setStartState(true))
  {
    display.println(F("error1"));
    display.display();      // Show initial text
    delay(1000);
    return;
  }

  // Wait for a bit longer than the conversion time.
  delay(2 + myUVSensor.getConversionTimeMillis());

  // Read UV values.
  if (ksfTkErrOk != myUVSensor.readAllUV())
  {
    display.println(F("error2"));
    display.display();      // Show initial text
    delay(1000);
    return;
  }

  display.println("A " + String(myUVSensor.getUVA()) + " B " + myUVSensor.getUVB() + " C " + myUVSensor.getUVC());
  
  display.display();      // Show initial text
  delay(1000);
}

void loop() {
  // put your main code here, to run repeatedly:
  showUV();
}
