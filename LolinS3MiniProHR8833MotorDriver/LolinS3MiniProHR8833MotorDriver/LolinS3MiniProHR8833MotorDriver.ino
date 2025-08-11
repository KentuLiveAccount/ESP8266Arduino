#include <Wire.h>
#include <LOLIN_I2C_MOTOR.h>
#include <Qmi8658c.h>

#define QMI_ADRESS 0x6b
#define QMI8658C_IIC_FREQUENCY 80*1000 //80kHz

Qmi8658c qmi8658c(QMI_ADRESS, QMI8658C_IIC_FREQUENCY);

/* QMI8658C configuration */
qmi8658_cfg_t qmi8658_cfg ={
  //.qmi8658_mode = qmi8658_mode_acc_only,
  //.qmi8658_mode = qmi8658_mode_dual,
  //.qmi8658_mode = qmi8658_mode_gyro_only,
  .qmi8658_mode = qmi8658_mode_dual,
  .acc_scale = acc_scale_2g,
  .acc_odr = acc_odr_1000,
  .gyro_scale = gyro_scale_256dps,
  .gyro_odr = gyro_odr_1000,
};

qmi8658_result_t qmi8658_result;

void setup1() 
{
  Serial.begin(9600);
  Serial.print("initializing QMI8658C\n");
}
void setup() 
{
  Serial.begin(9600);
  Serial.print("initializing QMI8658C\n");
  delay(1000);
  qmi8658_result = qmi8658c.open(&qmi8658_cfg);
  delay(1000);
  Serial.print("open result : ");
  Serial.println(qmi8658c.resultToString(qmi8658_result));
  Serial.print("deviceID = 0x");
  Serial.println(qmi8658c.deviceID, HEX);
  Serial.print("deviceRevisionID = 0x");
  Serial.println(qmi8658c.deviceRevisionID, HEX);
}

qmi_data_t data;

void printSign(float v)
{
  if (v < 0.0)
    Serial.print("-");
  else if (v > 0.0)
    Serial.print("+");
  else
    Serial.print("0");
}

void loop() {
  qmi8658c.read(&data);
  Serial.print("acc x: ");
  printSign(data.acc_xyz.x);
  Serial.print(" | ");
  Serial.print(" y: ");
  printSign(data.acc_xyz.y);
  Serial.print(" | ");
  Serial.print("z: ");
  printSign(data.acc_xyz.z);
  Serial.print(" gyro x: ");
  printSign(data.gyro_xyz.x);
  Serial.print(" | ");
  Serial.print(" y: ");
  printSign(data.gyro_xyz.y);
  Serial.print(" | ");
  Serial.print("z: ");
  printSign(data.gyro_xyz.z);
  Serial.println("");
  delay(10);
}

void loop2() {
  qmi8658c.read(&data);
  Serial.print("acc x: ");
  Serial.print(data.acc_xyz.x);
  Serial.print(" | ");
  Serial.print(" y: ");
  Serial.print(data.acc_xyz.y);
  Serial.print(" | ");
  Serial.print("z: ");
  Serial.print(data.acc_xyz.z);
  Serial.print(" gyro x: ");
  Serial.print(data.gyro_xyz.x);
  Serial.print(" | ");
  Serial.print(" y: ");
  Serial.print(data.gyro_xyz.y);
  Serial.print(" | ");
  Serial.print("z: ");
  Serial.print(data.gyro_xyz.z);
  Serial.println("");
  delay(10);
}

void loop1() {
  LOLIN_I2C_MOTOR motor; 
  motor.changeFreq(MOTOR_CH_BOTH, 40000); //Change A & B 's Frequency to 40kHz.
  motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CCW);
  motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CW);
  for (int i = 10; i <= 100; i += 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);
    delay(100);
  }
  delay(1000);
  
  for (int i = 100; i > 0; i -= 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);
    delay(100);
  }
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);
  delay(500);
  motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CW);
  motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CCW);
  motor.changeDuty(MOTOR_CH_A, 100);
  motor.changeDuty(MOTOR_CH_B, 100);
  delay(1000);
  
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);
  delay(500);

}
