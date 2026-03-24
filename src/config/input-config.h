#pragma once
#include <Arduino.h>

// --- INPUT DEFINITIONS ---

enum class InputType : uint8_t {
  NONE = 0,             // Channel is disabled / ignored
  SWITCH_3POS = 1,      // Evaluates Low, Mid, and High positions
};

enum InputServoMapping : uint8_t {
    NONE = 0,
    // Local Master Board Servos
    SRV_MASTER_1 = 1,
    SRV_MASTER_2 = 2,
    SRV_MASTER_3 = 3,
    SRV_MASTER_4 = 4,
    SRV_MASTER_5 = 5,
    SRV_MASTER_6 = 6,

    // Remote RS485 Bus Servos
    SRV_REMOTE_1 = 7,
    SRV_REMOTE_2 = 8,
    SRV_REMOTE_3 = 9,
    SRV_REMOTE_4 = 10,
    SRV_REMOTE_5 = 11,
    SRV_REMOTE_6 = 12
};

struct InputConfig {
  InputType type;          
  
  // --- Used for SWITCH_3POS ---
  // The targets (bits) to trigger based on the switch position
  uint32_t targetMaskLow;        // Triggered if value < thresholdLow
  uint32_t targetMaskMid;        // Triggered if value >= thresholdLow AND <= thresholdHigh
  uint32_t targetMaskHigh;       // Triggered if value > thresholdHigh
  
  // The limits defining the 3 switch zones
  // Units depend on input type:
  // For PWM: Raw value ranges 700-2300 (values ideally at 1300-1700)
  // For PPM: Raw value ranges 1000-2000 (values ideally at 1300-1700)
  // For SBus: Raw SBus units ranges from 0-2047 (values ideally at 900-1100)
  uint16_t thresholdLow;
  uint16_t thresholdHigh;

  // --- Used for PROPORTIONAL ---
  // Maps this input channel to an index in the global servo array (0 to 12); while 0 means off
  InputServoMapping targetServoIndex;
};