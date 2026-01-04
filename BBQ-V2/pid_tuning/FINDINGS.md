# BBQ Grill PID Analysis - Key Findings

**Date**: January 3, 2026  
**Dataset**: 260102.csv (2106 points, 4.2 hours of cooking data)

---

## Executive Summary

Analysis of real BBQ grill data revealed fundamental physics that explain why PID control produces bang-bang behavior. Built a physics-based combustion model that accurately simulates the grill's response, including a **2-3 minute oxygen saturation delay** that limits PID effectiveness.

**Bottom line**: Bang-bang control (0° ↔ 105° oscillation) is near-optimal given the system physics. PID optimization provides only 6.4% improvement over current parameters.

---

## Key Discoveries

### 1. Oxygen Saturation Effect
**Finding**: Charcoal combustion doesn't respond instantly when vent opens.

**Evidence from real data**:
- First 155 seconds after vent opens: Grill **cools** at -0.31°F/min (thermal inertia)
- After 155 seconds: Strong heating at +1.53°F/min (combustion ignited)
- **Absolute change**: +1.84°F/min acceleration

**Correlation analysis**:
- Time vs heating rate: **r=0.924** (very strong)
- Temperature vs heating rate: r=0.785 (strong, but secondary)

**Physical mechanism**:
1. Oxygen must penetrate porous charcoal bed
2. Surface layers heat to ignition temperature
3. Active combustion sites gradually build up
4. Reaches oxygen-flow-limited maximum (capped by vent geometry)

**Model**: `oxygen_saturation = 1 - exp(-t/78s)`
- τ = 78 seconds (time to reach 63% saturation)
- At t=155s: 86% saturated
- Asymptotically approaches 100% (never exceeds - physics ceiling)

### 2. Combustion Inertia
**Finding**: Actual combustion can't instantly track oxygen availability.

**Physical mechanism**:
- Oxygen already in grill chamber when vent closes
- Thermal mass of glowing charcoal bed
- Gradual changes in active combustion zone size

**Model**: `combustion_inertia = τ=90s`
- Smooths all changes in combustion level
- Prevents unrealistic instant response
- Combined with O₂ saturation = **~2 minute total delay**

### 3. Temperature-Dependent Combustion (Arrhenius Effect)
**Finding**: Hotter charcoal burns faster (positive feedback).

**Model**: `temp_factor = 0.5 + 0.5 × exp(0.3 × T_normalized × 4)`
- At 150-250°F: 1.0-1.5× multiplier
- At 400°F+: 2-3× multiplier (runaway potential)

### 4. Non-Linear Airflow Response
**Finding**: Intermediate servo positions are ineffective.

**Causes**:
- Vent geometry restricts flow below 15% opening (0-16°)
- Oxygen saturation delay makes 30-70° positions slow
- Only 90-105° provides quick, strong response

**Result**: System naturally prefers bang-bang operation.

---

## System Identification Results

### Heating Events (Servo ≥103° for 5+ minutes)
- **7 events** identified
- Average heating rate: **0.52°F/min**
- Range: 0.20 to 0.81°F/min
- Variability due to: temperature zone, time-since-opening

### Cooling Events (Servo ≤2° for 5+ minutes)
- **8 events** identified
- Average cooling rate: **-0.48°F/min**
- Range: -0.20 to -1.02°F/min
- Faster cooling at higher temperatures (Stefan-Boltzmann law)

---

## Combustion Model Parameters

### CombustionGrillSimulator (Final Calibrated Values)

**Base Combustion**:
- `base_rate = 1.8` → produces ~0.52°F/min heating at full airflow
- `thermal_mass = 50.0` → converts heat to temperature change

**Temperature Feedback**:
- `activation_energy = 0.3` → controls temperature sensitivity
- Arrhenius-style exponential acceleration with temperature

**Cold Start Behavior**:
- Below 120°F: 0.3× penalty (hard to ignite)
- 120-180°F: Linear ramp 0.3→1.0× (warming up)
- Above 180°F: 1.0× (normal combustion)

**Oxygen Saturation**:
- `oxygen_saturation_tau = 78s` → time constant for O₂ penetration
- Starts at 0 when vent opens
- Asymptotically approaches 1.0 (flow-limited ceiling)

**Combustion Inertia**:
- `combustion_tau = 90s` → time constant for combustion changes
- Prevents instant response
- Models thermal mass + residual oxygen

**Heat Loss**:
- `loss = 0.0008 × (temp_diff^1.25)` → calibrated to -0.48°F/min cooling
- Power law accounts for radiation + convection

**Airflow Restriction**:
- Below 15% servo opening: airflow × 0.3 (vent geometry)

---

## PID Optimization Results

### Current Parameters (Baseline)
- Kp = 13
- Ki = 20
- Kd = 15
- **Cost**: 13.54

### Optimized Parameters
- Kp = **27.99** (↑115% - more aggressive)
- Ki = **34.63** (↑73% - faster integral action)
- Kd = **20.04** (↑34% - better damping)
- **Cost**: 12.67
- **Improvement**: 6.4%

### Interpretation
Optimizer found slightly more aggressive parameters:
- Higher Kp: React faster when temperature deviates
- Higher Ki: Eliminate steady-state error faster
- Higher Kd: Better anticipate temperature changes

But **only 6.4% improvement** confirms that physics, not PID tuning, limits performance.

---

## Why PID Struggles With This System

### 1. Dead Time (2-3 minute delay)
PID's worst enemy. Controller opens vent → nothing happens for 2 minutes → PID increases output further → combustion finally kicks in → massive overshoot.

### 2. Non-Linear Servo Response
Intermediate positions (30-70°) are ineffective. System prefers extremes (0° or 105°).

### 3. Integral Windup
During the 2-minute dead time, integral term accumulates. By the time temperature moves, integral is huge → overshoot.

### 4. Positive Feedback
Hotter → faster combustion → hotter. Amplifies overshoots.

**Conclusion**: Bang-bang control is near-optimal given these constraints. PID can only optimize timing/amplitude, not eliminate oscillations.

---

## Model Validation

### Qualitative Match
✓ 2-minute delay before heating starts  
✓ Bang-bang behavior (0° ↔ 105° oscillation)  
✓ Non-linear servo response  
✓ Temperature-dependent heating rate  

### Quantitative Match
✓ Heating rate: ~0.52°F/min (matches real data)  
✓ Cooling rate: ~0.48°F/min (matches real data)  
✓ Oxygen saturation timeline: matches 155s transition  

---

## Recommendations

### For Current Hardware (ESP8266 + Servo)

**Option 1: Use Optimized PID**
- Kp=28, Ki=35, Kd=20
- Expect ±4-5°F oscillations (slightly tighter than current)
- Accept bang-bang as near-optimal

**Option 2: Tuned Bang-Bang**
- Simple hysteresis control
- Open at target-3°F, close at target+3°F
- May perform as well as PID with less computation

**Option 3: Predictive Control**
- Model Predictive Control (MPC) can explicitly handle 2-min delay
- Requires more computation (may need ESP32 upgrade)
- Potential for significant improvement

### For Hardware Upgrades

**Variable Speed Fan**:
- PWM-controlled blower instead of servo damper
- Faster response (no 2-min delay)
- Better continuous control
- Linear airflow relationship

**Secondary Temperature Sensor**:
- Charcoal bed temperature sensor
- Feedforward control: predict combustion intensity

---

## Next Steps

### Immediate
1. **Test optimized parameters** on real grill
2. **Collect new data** with Kp=28, Ki=35, Kd=20
3. **Compare** simulation vs reality

### Future Explorations
1. **Multiple cooking sessions** - validate model across conditions
2. **Disturbance modeling** - lid opening, meat loading, wind
3. **Oval vent geometry** - precise airflow vs servo angle mapping
   - Two metal plates with identical oval holes
   - Airflow = function of oval overlap area
   - Each degree of servo movement produces different airflow delta
   - Current model uses simple threshold (restricted below 15%)
   - True physics: non-linear geometric overlap function
   - Could measure actual airflow vs angle, fit polynomial/lookup table
4. **Alternative control** - MPC, adaptive, or bang-bang with learning

### Code Organization
1. Export `CombustionGrillSimulator` to separate `.py` module
2. Create production notebook with clean final models
3. Arduino code generator for optimized PID parameters

---

## References

**Data File**: `C:\GitHub\HaBBQVisual\script\260102.csv`  
**Analysis Notebook**: `grill_pid_analysis.ipynb`  
**Hardware**: Wemos D1 (ESP8266), MCP3202 ADC, Servo (0-105°)

**Key Analysis Cells**:
- Cell 14: Step response analysis (heating/cooling events)
- Cell 18: CombustionGrillSimulator class (physics-based model)
- Cell 21: Heating rate acceleration analysis (oxygen saturation discovery)
- Cell 25: PID optimization (differential_evolution)
