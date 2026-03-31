#pragma once

class GrillSimulator {
private:
  bool enabled;
  double simTemp;
  double simMeatTemp;
  unsigned long lastUpdateTime;
  
public:
  GrillSimulator() : enabled(false), simTemp(70.0), simMeatTemp(70.0), lastUpdateTime(0) {}
  
  void setEnabled(bool enable) {
    enabled = enable;
    if (enable && lastUpdateTime == 0) {
      lastUpdateTime = millis();
    }
  }
  
  bool isEnabled() {
    return enabled;
  }
  
  // Simulate grill thermal behavior based on servo angle
  void update(double servoAngle) {
    if (!enabled) return;
    
    unsigned long now = millis();
    double deltaTime = (now - lastUpdateTime) / 1000.0; // Convert to seconds
    if (lastUpdateTime == 0) deltaTime = 0.1; // First run
    lastUpdateTime = now;
    
    // Convert servo angle (0-105 degrees) to normalized air flow (0.0 to 1.0)
    double airFlow = servoAngle / 105.0;
    
    // Thermal model parameters (tune these to match your grill's behavior)
    const double maxHeatingRate = 15.0;      // Max degrees F per second at full airflow
    const double coolingRate = 2.0;          // Degrees F per second when cooling
    const double ambientTemp = 70.0;         // Ambient temperature
    const double charcoalMaxTemp = 700.0;    // Maximum temp charcoal can reach
    const double thermalInertia = 0.3;       // Thermal mass effect (0-1, lower = slower response)
    
    // Calculate target temperature based on air flow
    // More air = hotter (to a point), then too much air cools it
    double targetGrillTemp;
    if (airFlow < 0.7) {
      // Below 70% open: more air = hotter
      targetGrillTemp = ambientTemp + (charcoalMaxTemp - ambientTemp) * (airFlow / 0.7);
    } else {
      // Above 70%: too much air starts cooling
      targetGrillTemp = charcoalMaxTemp - (charcoalMaxTemp - 400) * ((airFlow - 0.7) / 0.3);
    }
    
    // Calculate temperature change
    double tempDiff = targetGrillTemp - simTemp;
    double changeRate;
    
    if (tempDiff > 0) {
      // Heating up
      changeRate = maxHeatingRate * airFlow * thermalInertia;
    } else {
      // Cooling down
      changeRate = -coolingRate * thermalInertia;
    }
    
    // Apply temperature change with damping
    double tempChange = changeRate * deltaTime;
    if (abs(tempChange) > abs(tempDiff)) {
      tempChange = tempDiff; // Don't overshoot
    }
    
    simTemp += tempChange;
    
    // Add some realistic noise
    simTemp += (random(-10, 10) / 10.0);
    
    // Meat temperature follows grill temp but slower
    double meatTempDiff = simTemp - simMeatTemp;
    simMeatTemp += meatTempDiff * 0.05 * deltaTime; // Slower thermal mass
    
    // Clamp values
    simTemp = constrain(simTemp, ambientTemp, charcoalMaxTemp);
    simMeatTemp = constrain(simMeatTemp, ambientTemp, 250.0);
  }
  
  double getGrillTemp() {
    return simTemp;
  }
  
  double getMeatTemp() {
    return simMeatTemp;
  }
};
