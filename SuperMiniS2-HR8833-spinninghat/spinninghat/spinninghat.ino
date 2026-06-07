#include <Wire.h>
#include <LOLIN_I2C_MOTOR.h>


void setup() 
{
  Serial.begin(9600);
  Serial.print("initializing\n");
  //Wire.begin(4, 5);   // SDA, SCL

  Wire.begin(2, 1);   // SDA, SCL
  pinMode(LED_BUILTIN, OUTPUT);
}


const int TOUCH_PIN = 7;
const int THRESHOLD = 40000;

#define MAXDUTY 50
void loop() {
  Serial.println(".");

  digitalWrite(LED_BUILTIN, HIGH);  // change state of the LED by setting the pin to the HIGH voltage level

  LOLIN_I2C_MOTOR motor; 
  motor.changeFreq(MOTOR_CH_BOTH, 40000); //Change A & B 's Frequency to 40kHz.
  motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CCW); // push
  motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CW); // pull
  for (int i = 10; i <= MAXDUTY; i += 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);
    delay(100);
  }
  delay(5000);
  
  for (int i = MAXDUTY; i > 0; i -= 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);
    delay(100);
  }
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);

  digitalWrite(LED_BUILTIN, LOW);  // change state of the LED by setting the pin to the HIGH voltage level

  delay(1000);
  
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);
  delay(500);

}
