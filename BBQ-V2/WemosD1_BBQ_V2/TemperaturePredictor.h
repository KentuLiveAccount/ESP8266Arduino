#pragma once

#include <stdint.h>

class TemperaturePredictor
{
public:
  static const int SampleCount = 7;
  static const uint32_t SampleIntervalMs = 5000;
  static const int HistorySeconds = 30;

  void Reset()
  {
    m_count = 0;
    m_next = 0;
    m_lastSampleTime = 0;
    m_slopePerSecond = 0.0;
  }

  void Add(uint32_t now, double temperature)
  {
    if (m_count > 0 && (uint32_t)(now - m_lastSampleTime) < SampleIntervalMs)
      return;

    m_times[m_next] = now;
    m_temperatures[m_next] = temperature;
    m_next = (m_next + 1) % SampleCount;
    if (m_count < SampleCount)
      m_count++;
    m_lastSampleTime = now;

    CalculateSlope();
  }

  bool IsReady() const
  {
    return m_count == SampleCount;
  }

  double Predict(double currentTemperature, int horizonSeconds) const
  {
    if (!IsReady())
      return currentTemperature;

    return currentTemperature + m_slopePerSecond * horizonSeconds;
  }

  double SlopePerMinute() const
  {
    return m_slopePerSecond * 60.0;
  }

private:
  void CalculateSlope()
  {
    if (m_count < 2)
    {
      m_slopePerSecond = 0.0;
      return;
    }

    const int oldest = m_count == SampleCount ? m_next : 0;
    const uint32_t referenceTime = m_times[(m_next + SampleCount - 1) % SampleCount];
    double sumX = 0.0;
    double sumY = 0.0;
    double sumXX = 0.0;
    double sumXY = 0.0;

    for (int i = 0; i < m_count; i++)
    {
      const int index = (oldest + i) % SampleCount;
      const double x = -(double)((uint32_t)(referenceTime - m_times[index])) / 1000.0;
      const double y = m_temperatures[index];
      sumX += x;
      sumY += y;
      sumXX += x * x;
      sumXY += x * y;
    }

    const double denominator = m_count * sumXX - sumX * sumX;
    if (denominator == 0.0)
    {
      m_slopePerSecond = 0.0;
      return;
    }

    m_slopePerSecond = (m_count * sumXY - sumX * sumY) / denominator;
  }

  uint32_t m_times[SampleCount] = {};
  double m_temperatures[SampleCount] = {};
  int m_count = 0;
  int m_next = 0;
  uint32_t m_lastSampleTime = 0;
  double m_slopePerSecond = 0.0;
};
