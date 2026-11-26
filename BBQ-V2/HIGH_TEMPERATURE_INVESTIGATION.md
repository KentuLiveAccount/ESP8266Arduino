# BBQ Controller High-Temperature Investigation

**Date:** November 25, 2026  
**Copilot session:** `01af677f-6695-45c3-ac24-dfdc1fd49b44`  
**Operating range reviewed:** 275-300°F

## Conclusion

The pit-temperature measurement circuit has adequate range and resolution for
275-300°F operation. The thermistor curve was characterized using measurements
at 32°F and boiling water (100°C), and the probe was tested in an oven at 325°F.
Therefore, 275-300°F is inside the probe's tested range rather than an
unverified high-temperature extrapolation.

The primary concerns at this temperature are controller overshoot and sensor
failure behavior, not thermistor or ADC range.

## Measurement Range

The controller uses an MCP3202 12-bit ADC and a measured 9,951.5592-ohm
reference resistor. Applying the ambient/pit-probe coefficients in
`WemosD1_BBQ_V2/WemosD1_BBQ_V2.ino` gives:

| Temperature | Thermistor resistance | ADC reading | Approximate resolution |
|---|---:|---:|---:|
| 225°F | 5,300 ohms | 2,673 | 0.07°F/count |
| 275°F | 2,570 ohms | 3,255 | 0.11°F/count |
| 300°F | 1,855 ohms | 3,453 | 0.15°F/count |
| 325°F | 1,367 ohms | 3,601 | 0.19°F/count |

At 300°F, approximately 643 ADC counts remain before full scale. The nested
running average further reduces conversion noise.

## Control Behavior

The existing grill analysis identifies a roughly 2-3 minute delay between
changing the vent and seeing the full combustion response. A large setpoint
increase from 225°F to 300°F will saturate the PID output and open the vent
fully. Combustion can continue increasing after the controller begins closing
the vent, so the first approach to the new target may overshoot.

The empirical control analysis was primarily based on lower-temperature
operation. High-temperature heat loss and combustion feedback may change the
steady-state vent behavior at 275-300°F.

For an initial cook:

1. Raise the target in stages instead of jumping directly from 225°F to 300°F.
2. Observe the first heating and cooling cycle before relying on steady control.
3. Keep an independent thermometer and high-temperature alarm active.

## Sensor-Failure Risk

The current conversion code maps invalid, open-circuit, or short-circuit probe
results to 0°C/32°F. The PID interprets that result as a cold grill and commands
more airflow, potentially opening the vent fully.

This is not fail-safe. A future controller change should detect implausible ADC
readings and rate-of-change faults, close the vent, and report a probe alarm
instead of passing the reading to the PID.

## Relevant Files

- `WemosD1_BBQ_V2/MCP3202.h`
- `WemosD1_BBQ_V2/ThermistorMath.h`
- `WemosD1_BBQ_V2/RunningAverage.h`
- `WemosD1_BBQ_V2/WemosD1_BBQ_V2.ino`
- `pid_tuning/FINDINGS.md`
- `pid_tuning/CONTROL_STRATEGIES.md`
