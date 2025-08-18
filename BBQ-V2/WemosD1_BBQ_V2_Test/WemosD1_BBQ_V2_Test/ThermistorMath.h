namespace Thermistor
{
struct ThermistorValues {
  int pin;
  double CM0;
  double CM1;
  double CM2;
};

double factor = 10000000.0;

float CelciusFromOhm(double ohm, const ThermistorValues &tv)
{
  if (ohm < 0)
    return 0;

  double lnR = log(ohm);

  // T(K) = 1/(C0 + C1Ln(R) + C2(Ln(R))^3)
  double temp  = factor /(tv.CM0 + lnR * tv.CM1 + tv.CM2 * lnR * lnR * lnR);
  temp  = temp - 273.15; // K -> C

  if (temp > 500 || temp < 0)
    temp = 0;

  return temp;
}

float farenheightFromCelsius(double c)
{
    return c * 9 / 5 + 32;
}
}