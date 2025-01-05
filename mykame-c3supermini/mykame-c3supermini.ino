/*
  My Kame
  
  ESP32C3 Super Mini with expansion board
  BluePad Lolin C3 Mini
  SG90 servos
  Staida game controller in BLE mode

  Servo library with local modification
  https://github.com/sturmfink/ESP32-ESP32S2-AnalogWrite?tab=readme-ov-file#servo-easing

  Wiring
*/

#include <Bluepad32.h>
#include "pwmWrite.h"

Pwm pwm = Pwm();

const int servoPin4 = 0;
const int servoPin5 = 5;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

struct throttle
{
  int throttleL = 0; // [-100, 100]
  int throttleR = 0; // [-100, 100]
};

throttle g_controller;

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

void setupBluePad()
{
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "axis L: %4d, %4d, axis R: %4d, %4d\n",
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY()       // (-511 - 512) right Y axis
    );
}

int toPercent(int in) // [-511, 512]
{
  int inCapped = max(-500, min(500, in));

  return inCapped * 100 / 500;
}

void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...
    if (ctl->a()) {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
            case 0:
                // Red
                ctl->setColorLED(255, 0, 0);
                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 0);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                break;
        }
        colorIdx++;
    }

    if (ctl->b()) {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x()) {
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
        // It is possible to set it by calling:
        // Some controllers have two motors: "strong motor", "weak motor".
        // It is possible to control them independently.
        ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                            0x40 /* strongMagnitude */);
    }

    g_controller.throttleL = toPercent(ctl->axisY());        // (-511 - 512) left Y axis
    g_controller.throttleR = toPercent(ctl->axisRY());       // (-511 - 512) right Y axis
    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    // dumpGamepad(ctl);
}

bool controllerConnected()
{
    for (auto myController : myControllers)
    {
        if (myController && myController->isConnected() && myController->isGamepad())
          return true;
    }

  return false;
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

const int elbowRF = 21;
const int elbowRB = 20;
const int elbowLF = 0;
const int elbowLB = 1;
const int shoulderRF = 10;
const int shoulderRB = 9;
const int shoulderLF = 2;
const int shoulderLB = 3;



// shoulders
// right: zero is towards back
// left:  zero is towards front

// elbows
// rf, lb: zero is up
// lf, rb: zero is down

// Servo sweep is 100 - 550, with 450 mapped to 180. 2 degree is split into 5 intervals
// "middle" is the neutral position on the robot
// for shoulder, this means middle of motion ranges 
// for elbow, this means foot naturally planted to the ground (also middle)

// Elbow: negative number is up, positive is down
// degree is absolute where 0 is middle

// Shoulder: negative number is front, positive is back
// degree is absolute where 0 is middle

void home()
{
  Serial.printf("home\n");
  pwm.writeServo(elbowRF, 90);  
  pwm.writeServo(elbowRB, 90);  
  pwm.writeServo(elbowLF, 90);  
  pwm.writeServo(elbowLB, 90);  
  pwm.writeServo(shoulderRF, 90);  
  pwm.writeServo(shoulderRB, 90);  
  pwm.writeServo(shoulderLF, 90);  
  pwm.writeServo(shoulderLB, 90);  
}

class CSmartServo
{
public:
  void Init(int iServo)
  {
    m_iServo = iServo;
    m_throttle = 100;

    if (m_iServo == shoulderLF)
      Serial.printf("construct: iServo: %d\n", m_iServo);

    //pinMode(m_iServo, OUTPUT);

    if (isShoulder())
      m_msecPerDegree = 5;
    else
      m_msecPerDegree = 0; //0 means no delay
  }

  void setThrottle(int throttle)
  {
      m_throttle = throttle;

      if (m_iServo == shoulderLF)
        Serial.printf("shoulderLF throttle %d\n", m_throttle);
  }

  int iServo() const { return m_iServo; }
  bool isShoulder() const
  {
    return (m_iServo == shoulderRF || m_iServo == shoulderRB || m_iServo == shoulderLF || m_iServo == shoulderLB);
  }
  /* 
    converts angle [-90, 90] to servo position [100, 550]
  */
  int angleToServoPos(int relAngle)
  {

    if (m_iServo == shoulderRF || m_iServo == shoulderRB)
      relAngle = relAngle * -1;
    else if (m_iServo == elbowLF || m_iServo == elbowRB)
      relAngle = relAngle * -1;

    return 90 + relAngle;
  }

  int servoPosToAngle(int pos)
  {
    int relAngle = pos - 90;

    if (m_iServo == shoulderRF || m_iServo == shoulderRB)
      relAngle = relAngle * -1;
    else if (m_iServo == elbowLF || m_iServo == elbowRB)
      relAngle = relAngle * -1;

    return relAngle;
  }

  // shoulder velocity
  // 40/200 = 4/20 = 1/5 - 1 degree every 5 millisec
  void moveToDirect(int relAngle)
  {
    relAngle = min(90, relAngle);
    relAngle = max(-90, relAngle);

    m_curAngle = relAngle;
    m_destAngle = relAngle;

    if (m_iServo == shoulderLF)
      Serial.printf("direct: shoulderLF throttle %d, angle %d\n", m_throttle, angleToServoPos(relAngle * m_throttle / 100));
    pwm.writeServo(m_iServo, angleToServoPos(relAngle * m_throttle / 100));
  }


  void moveTo(int relAngle)
  {
    relAngle = min(90, relAngle);
    relAngle = max(-90, relAngle);

    m_msecLast = millis();

    if (m_msecPerDegree > 0)
    {
      m_destAngle = relAngle;
    }
    else
    {
      m_curAngle = relAngle;
      m_destAngle = relAngle;

      if (m_iServo == shoulderLF)
        Serial.printf("moveTo: shoulderLF throttle %d, angle %d\n", m_throttle, angleToServoPos(relAngle * m_throttle / 100));
      pwm.writeServo(m_iServo, angleToServoPos(relAngle * m_throttle / 100));
    }
  }

  void nextGait()
  {
    if (isShoulder())
    {
      m_destAngle = -1 * m_destAngle;

      Serial.printf("nextGait iServo: %d, abs: %d\n", m_iServo, m_destAngle);
      return;
    }

    if (m_destAngle == 0)
      moveToDirect(-30);
    else
      moveToDirect(0);
  }

  // true means more motion remaining
  bool step()
  {
    if (m_throttle == 0)
    {
//      Serial.printf("zero throttle\n");
      return false;
    }

    int angleDelta = m_destAngle - m_curAngle;

    if (angleDelta == 0)
    {
//      Serial.printf("no more move\n");
      return false;
    }

    int msecCur = millis();
    int deltaMsec = msecCur - m_msecLast;

    if (angleDelta < 0)
      angleDelta = max(angleDelta, -1 * deltaMsec / m_msecPerDegree);
    else
      angleDelta = min(angleDelta, deltaMsec / m_msecPerDegree);

    if (angleDelta == 0)
    {
//      Serial.printf("not time yet\n");
      return true;
    }

    m_msecLast += abs(angleDelta) * m_msecPerDegree;
    m_curAngle += angleDelta;

    if (angleDelta > 0 && m_curAngle > m_destAngle)
      m_curAngle = m_destAngle;
    if (angleDelta < 0 && m_curAngle < m_destAngle)
      m_curAngle = m_destAngle;

    if (m_iServo == shoulderLF)
      Serial.printf("step: shoulderLF throttle %d, angle %d\n", m_throttle, angleToServoPos(m_curAngle * m_throttle / 100));

    pwm.writeServo(m_iServo, angleToServoPos(m_curAngle * m_throttle / 100));

    if (m_curAngle != m_destAngle)
    {
//      Serial.printf("more move left\n");
      return true;
    }
    else
    {
//      Serial.printf("moved to the end\n");
      return false;
    }
  }

private:
  int m_iServo;
  int m_curAngle;
  int m_destAngle;
  int m_msecDuration;
  int m_msecLast;
  int m_msecPerDegree;
  int m_throttle; // [-100, 100]
};

class CKame
{
public:
  void Init()
  {
     m_rgSv[0].Init(elbowRF);
     m_rgSv[1].Init(elbowRB);
     m_rgSv[2].Init(elbowLF);
     m_rgSv[3].Init(elbowLB);
     m_rgSv[4].Init(shoulderRF);
     m_rgSv[5].Init(shoulderRB);
     m_rgSv[6].Init(shoulderLF);
     m_rgSv[7].Init(shoulderLB);
  }
  void readyStepForward()
  {
    for (int i = 0; i < 8; i++)
    {
      switch (m_rgSv[i].iServo())
      {
        case elbowRF:
        case elbowLB:
          m_rgSv[i].moveToDirect(-30);
          break;
        case elbowLF:
        case elbowRB:
          m_rgSv[i].moveToDirect(0);
          break;

        case shoulderLB:
        case shoulderRF:
          m_rgSv[i].moveToDirect(20); //right front sholuder back
          m_rgSv[i].moveTo(-20);
          break;
        case shoulderLF:
        case shoulderRB:
          m_rgSv[i].moveToDirect(-20);//right back shoulder forward
          m_rgSv[i].moveTo(20);
      }
    }
  }

  void rest()
  {
    if (!m_readyToWalk)
      return;

    Serial.printf("rest\n");

    for (int i = 0; i < 8; i++)
    {
      if (!m_rgSv[i].isShoulder())
        m_rgSv[i].moveToDirect(0);
    }

    m_readyToWalk = false;
  }

  void ensureReadyToWalk()
  {
    if (m_readyToWalk)
      return;

    Serial.printf("walk\n");

    readyStepForward();
    m_readyToWalk = true;
  }

  void updateThrottle(int throttleL, int throttleR)
  {

   Serial.printf("throttle L: %d, R: %d\n", throttleL, throttleR);

    for (int i = 0; i < 8; i++)
    {
      switch (m_rgSv[i].iServo())
      {
        case shoulderLF:
        case shoulderLB:
          m_rgSv[i].setThrottle(throttleL);
          break;
        case shoulderRF:
        case shoulderRB:
          m_rgSv[i].setThrottle(throttleR);
      }
    }

    if (throttleL != 0 || throttleR != 0)
      ensureReadyToWalk();
    else
      rest();
  }

  void onLoopIteration()
  {
    if (!m_readyToWalk)
      return;

    bool fContinue = false;
    for (int i = 0; i < 8; i++)
      fContinue |= m_rgSv[i].step();

    if (fContinue)
      return;
      
    for (int i = 0; i < 8; i++)
      m_rgSv[i].nextGait();
    
    Serial.printf("\n");
  }

  bool m_readyToWalk = false;
  CSmartServo m_rgSv[8];
} g_kame;


/* 
  gait template 

elbowRF       0, -30, -30,   0,   0, 
elbowRB       0,   0,   0, -30, -30,
elbowLF       0,   0,   0, -30, -30,
elbowLB       0, -30, -30,   0,   0,
shoulderRF   20, nil, -20, nil,  20,
shoulderRB  -20, nil,  20, nil, -20,
shoulderLF  -20, nil,  20, nil, -20,
shoulderLB   20, nil, -20, nil,  20,
*/




void setup() {
  Serial.begin(115200);
  Serial.println("hi");

  setupBluePad();

  home();

  g_kame.Init();

  delay(1000);
  g_kame.ensureReadyToWalk();
  delay(1000);
  g_kame.rest();
  delay(1000);
}

void loop()
{
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();

    if (controllerConnected())
    {
        if (dataUpdated)
        {
          processControllers();
          g_kame.updateThrottle(g_controller.throttleL * -1, g_controller.throttleR * -1);
        }

        g_kame.onLoopIteration();

        delay(1);
    }
    else
    {
        delay(100);
    }
}