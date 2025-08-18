
template <int c> class RunningAverage
{
public:
double CurValue() { return rgv[(c + m_i - 1) % c];}
bool HasAverage(){ return m_hasavg;}
double Average() { return (double)m_sum / (double)c;}
void Add(int v)
{
  if (!m_hasavg)
  {
    m_sum = m_sum + v;
    rgv[m_i] = v;
    m_i++;
    if (m_i == c)
    {
      m_hasavg = true;
      m_i = 0;
    }
  }
  else
  {
    m_sum = m_sum + v - rgv[m_i];
    rgv[m_i] = v;
    m_i = (m_i + 1) % c;
  }
}
private:

int rgv[c]={};
int m_sum = 0;
bool m_hasavg = false;
int m_i = 0;
};

template <int cfirst, int csecond> class NestedRunningAverage
{
public:
  bool HasAverage(){ return m_second.HasAverage();}
  double Average() { return m_second.Average();}
  double AverageInner() { return m_first.Average();}
  double CurValue() { return m_second.CurValue();}
  double CurValueInner() { return m_first.CurValue();}
  void Add(int v)
  {
    m_first.Add(v);
    if (m_first.HasAverage())
    {
      //Serial.printf("%f\n", );
      m_second.Add(floor(m_first.Average() + 0.5));
    }
  }
private:
  RunningAverage<cfirst> m_first;
  RunningAverage<csecond> m_second;
};