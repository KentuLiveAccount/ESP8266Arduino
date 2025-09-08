#pragma once

const int cDataMax = 100;
int temps[cDataMax];
int intTemps[cDataMax];
int target[cDataMax];
int angles[cDataMax];
double pidPs[cDataMax];
double pidIs[cDataMax];
double pidDs[cDataMax];
int cData = 0;

void AddTemp(int temp, int tempInt, int targetTemp, int angleI, double pidP, double pidI, double pidD)
{
  const int i = cData++ % cDataMax;

  temps[i] = temp;
  intTemps[i] = tempInt;
  target[i] = targetTemp;
  angles[i] = angleI;
  pidPs[i] = pidP;
  pidIs[i] = pidI;
  pidDs[i] = pidD;
}

int currentIndex()
{
  return (cData + cDataMax - 1) % cDataMax;
}

void UpdateCurrentTarget(int newTarget)
{
  target[currentIndex()] = newTarget;
}

String ReadCur()
{
  const int i = currentIndex();

  String str = "";
  str += "{ \"currenttemp\": ";
  str += String(temps[i]);
  str += ", \"internatemp\": ";
  str += String(intTemps[i]);
  str += ", \"targettemp\": ";
  str += String(target[i]);
  str += ", \"servoangle\": ";
  str += String(angles[i]);
  str += "}";

  return str;
}

String ReadCurPID()
{
  const int i = currentIndex();

  String str = "{ \"currenttemp\": ";
  str += String(temps[i]);
  str += ", \"internatemp\": ";
  str += String(intTemps[i]);
  str += ", \"targettemp\": ";
  str += String(target[i]);
  str += ", \"servoangle\": ";
  str += String(angles[i]);
  str += ", \"pidp\": ";
  str += String(pidPs[i]);
  str += ", \"pidi\": ";
  str += String(pidIs[i]);
  str += ", \"pidd\": ";
  str += String(pidDs[i]);
  str += "}";

  return str;
}

template <class T> void SendTemps(T &server)
{
  int count = cData < cDataMax ? cData : cDataMax;
  int i = cData < cDataMax ? 0 : cData;
  server.sendContent("{\"message\": [");

  while (count)
  {
    server.sendContent("{ \"currenttemp\": ");
    server.sendContent(String(temps[i % cDataMax]));
    server.sendContent(", \"internatemp\": ");
    server.sendContent(String(intTemps[i % cDataMax]));
    server.sendContent(", \"targettemp\": ");
    server.sendContent(String(target[i++ % cDataMax]));

    if (count > 1)
      server.sendContent("}, ");
    else
      server.sendContent("}");
    count--;
  }

  server.sendContent("]}");
}

template <class T> void SendTempsPID(T &server)
{
  int count = cData < cDataMax ? cData : cDataMax;
  int i = cData < cDataMax ? 0 : cData;
  server.sendContent("{\"message\": [");

  while (count)
  {
    server.sendContent("{ \"currenttemp\": ");
    server.sendContent(String(temps[i % cDataMax]));
    server.sendContent(", \"internatemp\": ");
    server.sendContent(String(intTemps[i % cDataMax]));
    server.sendContent(", \"targettemp\": ");
    server.sendContent(String(target[i % cDataMax]));
    server.sendContent(", \"angle\": ");
    server.sendContent(String(angles[i % cDataMax]));
    server.sendContent(", \"pidp\": ");
    server.sendContent(String(pidPs[i % cDataMax]));
    server.sendContent(", \"pidi\": ");
    server.sendContent(String(pidIs[i % cDataMax]));
    server.sendContent(", \"pidd\": ");
    server.sendContent(String(pidDs[i++ % cDataMax]));

    if (count > 1)
      server.sendContent("}, ");
    else
      server.sendContent("}");
    count--;
  }

  server.sendContent("]}");
}

/*
json
 13  {"message": [
60  {"currenttemp": 000, "internatemp": 000, "targettemp": 000},
  2  ]}

jsonpid
 13  {"message": [
119  {"currenttemp": 000, "internatemp": 000, "targettemp": 000, "angle": 000, "pidP": 00.00, "pidI": 00.00, "pidD": 00.00},
  2  ]}

  11900 + 15 = 11,915 bytes
*/
