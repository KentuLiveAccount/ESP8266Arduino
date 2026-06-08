
#include <Wire.h>
#include <LOLIN_I2C_MOTOR.h>

#define TOUCH_PIN 7
#define MAXDUTY   60

bool motorEnabled = false;

// -----------------------------
// TOUCH CALIBRATION
// -----------------------------
uint32_t touchBaseline = 0;
uint32_t pressThreshold = 0;
uint32_t releaseThreshold = 0;

void calibrateTouch() {
  Serial.println("Calibrating touch... don't touch!");

  digitalWrite(LED_BUILTIN, HIGH);

  delay(1000);

  uint64_t sum = 0;

  for (int i = 0; i < 20; i++) {
    sum += touchRead(TOUCH_PIN);
    delay(20);
  }

  touchBaseline = sum / 20;

  // ESP32-S3: touch increases value
  pressThreshold   = touchBaseline + 12000;
  releaseThreshold = touchBaseline + 6000;

  Serial.printf("Baseline: %lu\n", touchBaseline);
  Serial.printf("Press: %lu\n", pressThreshold);
  Serial.printf("Release: %lu\n", releaseThreshold);

  digitalWrite(LED_BUILTIN, LOW);

}

// -----------------------------
// TOUCH CHECK (debounced)
// -----------------------------
bool checkTouch() {
  static int count = 0;
  static bool latched = false;

  int v = touchRead(TOUCH_PIN);

  // Debug if needed:
  // Serial.println(v);

  if (!latched && v > pressThreshold) {
    count++;
    if (count >= 3) {
      count = 0;
      latched = true;
      motorEnabled = !motorEnabled;
      return true;
    }
  } else {
    count = 0;
  }

  // wait for release before next press
  if (latched && v < releaseThreshold) {
    latched = false;
  }

  return false;
}

// -----------------------------
// SAFE STOP (always consistent)
// -----------------------------
void stopMotor(LOLIN_I2C_MOTOR &motor) {

  motor.changeDuty(MOTOR_CH_A, 0);
  motor.changeDuty(MOTOR_CH_B, 0);
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);
  digitalWrite(LED_BUILTIN, LOW);
}

// -----------------------------
// SMART DELAY
// -----------------------------
void smartDelay(int ms) {
  unsigned long start = millis();

  while (millis() - start < ms) {

    if (checkTouch())
      return; // motor state has changed

    delay(5);
  }
}

void testMotor()
{
  LOLIN_I2C_MOTOR motorl; 
  motorl.changeFreq(MOTOR_CH_BOTH, 40000);
  motorl.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CCW);
  motorl.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CW);

  // Ramp up
  for (int i = 10; i <= MAXDUTY; i += 10)
  {
    motorl.changeDuty(MOTOR_CH_A, i);
    motorl.changeDuty(MOTOR_CH_B, i);

    delay(100);
  }

  // Hold
  delay(1000);

  // Ramp down
  for (int i = MAXDUTY; i > 0; i -= 10)
  {
    motorl.changeDuty(MOTOR_CH_A, i);
    motorl.changeDuty(MOTOR_CH_B, i);

    delay(100);
  }

  motorl.changeDuty(MOTOR_CH_A, 0);
  motorl.changeDuty(MOTOR_CH_B, 0);
  motorl.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);
}

// -----------------------------
// SETUP
// -----------------------------
void setup() {
  Serial.begin(9600);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Wire.begin(2, 1); // SDA, SCL

  delay(500);

  calibrateTouch(); // ✅ auto threshold

  Serial.println("Ready. Touch to toggle.");
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {

  LOLIN_I2C_MOTOR motor; 

  // Ensure OFF state is always enforced
  if (!motorEnabled) {
    stopMotor(motor);
    smartDelay(300); // small guard
    return;
  }

  // -------------------------
  // YOUR ORIGINAL LOGIC
  // -------------------------

  digitalWrite(LED_BUILTIN, HIGH);

  motor.changeFreq(MOTOR_CH_BOTH, 40000);
  motor.changeStatus(MOTOR_CH_A, MOTOR_STATUS_CCW);
  motor.changeStatus(MOTOR_CH_B, MOTOR_STATUS_CW);

  // Ramp up
  for (int i = 10; motorEnabled && i <= MAXDUTY; i += 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);

    smartDelay(100);
  }

  if (!motorEnabled) return;

  // Hold
  smartDelay(4000);

  if (!motorEnabled) return;

  // Ramp down
  for (int i = MAXDUTY; motorEnabled && i > 0; i -= 10)
  {
    motor.changeDuty(MOTOR_CH_A, i);
    motor.changeDuty(MOTOR_CH_B, i);

    smartDelay(100);
  }

  motor.changeDuty(MOTOR_CH_A, 0);
  motor.changeDuty(MOTOR_CH_B, 0);
  motor.changeStatus(MOTOR_CH_BOTH, MOTOR_STATUS_STOP);

  smartDelay(1000);
}
