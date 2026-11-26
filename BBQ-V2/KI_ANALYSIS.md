# Integral Gain (Ki) Analysis

**Study session:** `7d3db78e-1124-4d5e-b61f-d3aeea68eee8`  
**Context name:** Prepare Online Cooking Session  
**Firmware commit studied:** `7fb966d` (`Add predictive PID controls`)  
**Archived source note:** `C:\GitHub\bbq_tracker\data\20260912\controller-study.md`

## Current Conclusion

The current `Ki=20` integrates much faster than the charcoal grill responds.
However, reducing Ki to `0.5` is not recommended because replay testing showed
that it delayed useful corrections enough to likely worsen temperature
overshoot and undershoot.

Temperature tracking is a higher priority than merely reducing the amount of
time the servo spends at its limits. The next controlled test should begin with
a moderate `Ki` around **5-10**, combined with conditional anti-windup and
integral separation. Keep `Kp=13` and `Kd=15` unchanged initially so the effect
of the integral changes can be isolated.

## Why Ki=20 Is Too Aggressive

Arduino PID_v1 uses a 100 ms default sample period. With `Ki=20`, a persistent
1°F error changes the integral output by approximately 20 PID output units per
second. The integral can therefore traverse the complete 0-255 output range in
about 13 seconds.

Measured grill response delay was approximately 84-115 seconds. The integral
can saturate many times faster than the grill can show the result of an airflow
change. This causes the controller to retain maximum or minimum output while
the delayed combustion response continues, contributing to overshoot and
rail-to-rail cycling.

At a true steady state:

```text
error = 0
P = 0
D = 0
output = I
```

The integral term is still needed to retain the intermediate damper angle that
holds the target temperature. The goal is to stop it from accumulating
unhelpfully during warm-up or output saturation, not to remove it entirely.

## September 2026 Replay

Recorded predicted-temperature input from the clean 150°F test was replayed
through the PID_v1 algorithm. Replaying `Ki=20` matched the recorded servo
output within 0.5 degrees mean error, validating the replay for comparing
controller output.

This was not a complete closed-loop grill simulation: changing the replayed
servo output would change subsequent combustion and temperature, while the
recorded temperature trace remains fixed.

| Ki | Replay result |
|---:|---|
| 0.5 | Reduced rail time, but reacted too late and would likely worsen temperature excursions |
| 5 | Preserved the first closure closely, but delayed a later correction by about 12 seconds |
| 10 | Preserved most original response timing while being somewhat less abrupt |
| 20 | Reacted fastest, but rapidly reversed saturated integral state and contributed to rail-to-rail cycling |

## Recommended Controller Changes

1. Expose the individual P, I, and D components in telemetry.
2. Add conditional anti-windup: do not integrate farther into an active output
   limit.
3. Enable integral action only near the target and/or while temperature slope
   is modest.
4. Start controlled testing with `Ki=5-10`.
5. Keep prediction in shadow mode during the first anti-windup test.
6. Preserve a nonzero minimum physical damper opening so an extended minimum
   command does not extinguish the charcoal.

## Older Optimization Results

The repository contains several older optimization outputs:

| Artifact | Reported Ki |
|---|---:|
| `pid_tuning/optimal_pid.txt` | 21.01 |
| `pid_tuning/FINDINGS.md` | 34.63, rounded recommendation 35 |
| Saved notebook export output | 31.74 |

These values came from different states of the simulation and objective
function. The notebook contains saved outputs from multiple runs, so they
should not be treated as one consistent final recommendation. They also
optimize a model rather than directly addressing the integral saturation seen
in the September controller data.

The September replay and live observations supersede those values for the next
firmware experiment: test anti-windup and integral separation first, using a
moderate `Ki=5-10`.

## Data Quality

For analysis of the September cook, exclude the interval from
`2026-09-12 22:13:00 -07:00` through `2026-09-13 01:49:55 -07:00`. During that
period the ambient probe touched meat and then the grate, so it did not
represent chamber-air temperature.
