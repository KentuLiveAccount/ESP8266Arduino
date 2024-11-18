#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

// ESP32C3 with PCA9685 servo driver breakout
/*
  Wiring

  ESP32C3   PCA9685
  GND    -> GND
  3V3    -> VCC
  GPIO5  -> SDA
  GPIO4  -> SCL
  3V3    -> OE (via switch: ON -> NO POWER to Device, leave open to power device)

  External power (5V) to V+ and GND to power devices

  Note: observing occasional erratic servo movement. supplying separate power to sservo does not help
*/

Adafruit_PWMServoDriver pwm1 = Adafruit_PWMServoDriver(0x40, Wire);

const int sdaPin = 5;
const int sclPin = 4;

const int elbowRF = 0;
const int elbowRB = 1;
const int elbowLF = 2;
const int elbowLB = 3;
const int shoulderRF = 4;
const int shoulderRB = 5;
const int shoulderLF = 6;
const int shoulderLB = 7;

void setup() {
  Wire.setPins(sdaPin, sclPin);
  Serial.begin(9600);
  Serial.println("16 channel PWM test!");

  pwm1.begin();
  pwm1.setPWMFreq(50);  // SG90 operates at 50Hz
}

void fullSweepForever()
{
  // servo sweep from 0 to 180 maps to 100 - 550 (changes with frequencies)
  for (int i = 100; i < 551; i += 5)
  {
    Serial.printf("pwm 0: 0, %d\n", i);
    pwm1.setPWM(0, 0, i);
    delay(300);
  }

}

void nearZeroTest()
{
    pwm1.setPWM(0, 0, 100); // zero is 100
    delay(1000);
    pwm1.setPWM(0, 0, 550); // 180 is 550
    delay(1000);

  int v = 90;
  for (;;)
  {
    Serial.printf("pwm 0: 0, %d\n", v);
    pwm1.setPWM(0, 0, v);
    delay(300);
    if (v == 90) v = 95;
    else if (v == 95) v = 100;
    else if (v == 100) v = 105;
    else if (v == 105) v = 90;
  }
}

void near180Test()
{
  int v = 545;
  for (;;)
  {
    Serial.printf("pwm 0: 0, %d\n", v);
    pwm1.setPWM(0, 0, v);
    delay(300);
    if (v == 545) v = 550;
    else if (v == 550) v = 555;
    else if (v == 555) v = 560;
    else if (v == 560) v = 545;
  }
}


void sweepAnd90(int servo)
{
    pwm1.setPWM(servo, 0, 100); // zero is 100
    delay(1000);
    pwm1.setPWM(servo, 0, 550); // 180 is 550
    delay(1000);
    pwm1.setPWM(servo, 0, 325); // 90 is 325 (100 + 450 / 2)
    delay(1000);
}

void sweepAndRestAt90()
{
  sweepAnd90 (0);
}

void sweepAndRestAt90Elbows()
{
  sweepAnd90 (elbowRF);
  sweepAnd90 (elbowRB);
  sweepAnd90 (elbowLF);
  sweepAnd90 (elbowLB);
}

// shoulders
// right: zero is towards back
// left:  zero is towards front

// elbows
// rf, lb: zero is up
// lf, rb: zero is down

// sweep is 100 - 550, with 450 mapped to 180. 2 degree is split into 5 intervals
// "middle" is the newutral
// for shoulder, this means middle of motion ranges 
// for elbow, this means foot naturally planted to the ground (also middle)

void sweeplessAnd90(int servo)
{
    pwm1.setPWM(servo, 0, 212); // zero is 100
    delay(1000);
    pwm1.setPWM(servo, 0, 438); // 180 is 550
    delay(1000);
    pwm1.setPWM(servo, 0, 325); // 90 is 325 (100 + 450 / 2)
    delay(1000);
}

// moves elbows. negative number is up, positive is down
// degree is absolute where 0 is middle
void moveElbowTo(int servo, int degree)
{
  degree = min(90, degree);
  degree = max(-90, degree);

  if (servo == elbowLF || servo == elbowRB)
    degree = degree * -1;

  pwm1.setPWM(servo, 0, 325 + (degree * 5 / 2));
}

// moves shoulders. negative number is front, positive is back
// degree is absolute where 0 is middle
void moveShoulderTo(int servo, int degree)
{
  degree = min(90, degree);
  degree = max(-90, degree);

  if (servo == shoulderRF || servo == shoulderRB)
    degree = degree * -1;

  pwm1.setPWM(servo, 0, 325 + (degree * 5 / 2));
}

void raiseElbowsAndRest()
{
  moveElbowTo(elbowRF, -30);
  delay(1000);
  moveShoulderTo(shoulderRF, 0);
  delay(1000);
  moveElbowTo(elbowRB, -30);
  delay(1000);
  moveShoulderTo(shoulderRB, 0);
  delay(1000);
  moveElbowTo(elbowLF, -30);
  delay(1000);
  moveShoulderTo(shoulderLF, 0);
  delay(1000);
  moveElbowTo(elbowLB, -30);
  delay(1000);
  moveShoulderTo(shoulderLB, 0);
  delay(1000);
}

void sweepAndRestAt90Shoulders()
{
  sweeplessAnd90 (shoulderRF);
  sweeplessAnd90 (shoulderRB);
  sweeplessAnd90 (shoulderLF);
  sweeplessAnd90 (shoulderLB);
}


void home()
{
  pwm1.setPWM(elbowRF, 0, 325);  
  pwm1.setPWM(elbowRB, 0, 325);  
  pwm1.setPWM(elbowLF, 0, 325);  
  pwm1.setPWM(elbowLB, 0, 325);  
  pwm1.setPWM(shoulderRF, 0, 325);  
  pwm1.setPWM(shoulderRB, 0, 325);  
  pwm1.setPWM(shoulderLF, 0, 325);  
  pwm1.setPWM(shoulderLB, 0, 325);  
}

class CSmartServo
{
public:
  CSmartServo(int iServo):m_iServo(iServo){}

  int relToAbs(int relAngle)
  {
    relAngle = min(90, relAngle);
    relAngle = max(-90, relAngle);

    if (m_iServo == shoulderRF || m_iServo == shoulderRB)
      relAngle = relAngle * -1;
    else if (m_iServo == elbowLF || m_iServo == elbowRB)
      relAngle = relAngle * -1;

    return 325 + (relAngle * 5 / 2);
  }

  void SetCurAngle(int relAngle)
  {
    m_iCurrentPosAbs = relToAbs(relAngle);
    m_iDestinationPosAbs = m_iCurrentPosAbs;
  }

  void moveTo(int relAngle, int msecDuration)
  {
    relAngle = min(90, relAngle);
    relAngle = max(-90, relAngle);

    if (m_iServo == shoulderRF || m_iServo == shoulderRB)
      relAngle = relAngle * -1;
    else if (m_iServo == elbowLF || m_iServo == elbowRB)
      relAngle = relAngle * -1;

    int absPos = 325 + (relAngle * 5 / 2);

    m_msecLast = millis();
    m_msecDuration = msecDuration;

    if (m_msecDuration > 0)
    {
      m_iDestinationPosAbs = absPos;
    }
    else
    {
      m_iCurrentPosAbs = absPos;
      m_iDestinationPosAbs = absPos;
      pwm1.setPWM(m_iServo, 0, m_iCurrentPosAbs);
    }
  }

  bool loop()
  {
    if (m_iCurrentPosAbs == m_iDestinationPosAbs)
      return false;

    int msecCur = millis();

    int deltaMsec = msecCur - m_msecLast;
    m_msecLast = msecCur;

    if (m_msecDuration == deltaMsec)
    {
      m_msecDuration = 0;
      m_iCurrentPosAbs = m_iDestinationPosAbs;
    }
    else
    {
      int posDelta = m_iDestinationPosAbs - m_iCurrentPosAbs;

      if (posDelta == 0)
        return false;

      posDelta = posDelta * deltaMsec / m_msecDuration;
      m_msecDuration -= deltaMsec;

      if (posDelta == 0)
        return true;

      m_iCurrentPosAbs += posDelta;

      if (posDelta > 0 && m_iCurrentPosAbs > m_iDestinationPosAbs)
        m_iCurrentPosAbs = m_iDestinationPosAbs;
      if (posDelta < 0 && m_iCurrentPosAbs < m_iDestinationPosAbs)
        m_iCurrentPosAbs = m_iDestinationPosAbs;
    }

    pwm1.setPWM(m_iServo, 0, m_iCurrentPosAbs);
    return true;
  }
private:
  int m_iServo;
  int m_iCurrentPosAbs;
  int m_iDestinationPosAbs;
  int m_msecDuration;
  int m_msecLast;
};

CSmartServo m_rgSv[8] = {elbowRF, elbowRB, elbowLF, elbowLB, shoulderRF, shoulderRB, shoulderLF, shoulderLB};

void finishMove()
{
  bool fContinue = true;

  while (fContinue)
  {
    fContinue = false;
    for (int i = 0; i < 8; i++)
      fContinue |= m_rgSv[i].loop();

    delay(5);
  }
}

void readyStepForward()
{
  m_rgSv[elbowRF].moveTo(0, 0 /* msecDur */);
  m_rgSv[elbowLF].moveTo(0, 0 /* msecDur */);
  m_rgSv[elbowRB].moveTo(0, 0 /* msecDur */);
  m_rgSv[elbowLB].moveTo(0, 0 /* msecDur */);

  m_rgSv[shoulderRF].moveTo(20, 0 /* msecDur */);
  m_rgSv[shoulderLB].moveTo(20, 0 /* msecDur */);
  m_rgSv[shoulderLF].moveTo(-20, 0 /* msecDur */);
  m_rgSv[shoulderRB].moveTo(-20, 0 /* msecDur */);
}

void stepForwardInPair()
{
  int elbowLift = -30;
  int msecDuration = 200;
  int shoulderForward = -20;
  int shoulderBack = 20;

  m_rgSv[elbowRF].moveTo(elbowLift, 0 /* msecDur */);
  m_rgSv[elbowLB].moveTo(elbowLift, 0 /* msecDur */);
  delay(50);

  m_rgSv[shoulderRF].moveTo(shoulderForward, msecDuration);
  m_rgSv[shoulderLB].moveTo(shoulderForward, msecDuration);
  m_rgSv[shoulderLF].moveTo(shoulderBack, msecDuration);
  m_rgSv[shoulderRB].moveTo(shoulderBack, msecDuration);
  finishMove();

  delay(50);
  m_rgSv[elbowRF].moveTo(0, 0 /* msecDur */);
  m_rgSv[elbowLB].moveTo(0, 0 /* msecDur */);

  delay(200);

  m_rgSv[elbowLF].moveTo(elbowLift, 0 /* msecDur */);
  m_rgSv[elbowRB].moveTo(elbowLift, 0 /* msecDur */);
  delay(50);

  m_rgSv[shoulderLF].moveTo(shoulderForward, msecDuration);
  m_rgSv[shoulderRB].moveTo(shoulderForward, msecDuration);
  m_rgSv[shoulderRF].moveTo(shoulderBack, msecDuration);
  m_rgSv[shoulderLB].moveTo(shoulderBack, msecDuration);
  finishMove();

  delay(50);
  m_rgSv[elbowLF].moveTo(0, 0 /* msecDur */);
  m_rgSv[elbowRB].moveTo(0, 0 /* msecDur */);

  delay(200);
}

void loop()
{
  home();
  delay(1000);
  readyStepForward();
  delay(1000);
  //raiseElbowsAndRest();
  //delay(1000);

  for (int i = 0; i < 10; i++)
    stepForwardInPair();

  for (;;)
  {
    delay(300);
  }
}