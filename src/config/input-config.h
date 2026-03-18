#pragma once
#include <Arduino.h>

// --- INPUT DEFINITIONS ---

enum class InputType : uint8_t {
  NONE = 0,             // Channel is disabled / ignored
  SWITCH_3POS = 1,      // Evaluates Low, Mid, and High positions
  PROPORTIONAL = 2      // Raw value passed through (e.g., for servos/ESC)
};

struct InputConfig {
  InputType type;          
  
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
};

struct LightInputChannel {
	// Pin for Reverse Signal from External Controller
	uint8_t reverseSignal;
	// Pin for Brake Signal from External Controller
	uint8_t brakeSignal;
};