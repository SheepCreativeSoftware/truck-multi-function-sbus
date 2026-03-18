#pragma once
#include <Arduino.h>
#include "input-config.h"
#include "local-output-config.h"

#define NUM_SBUS_CHANNELS 16
#define NUM_PPM_CHANNELS 8
#define NUM_LOCAL_OUTPUTS 14

// Defines how the physical PPM pin is evaluated
enum class PpmInputMode : uint8_t {
  MULTIPLEXED_8CH = 0, // Expects a standard 8-channel PPM stream
  SINGLE_PWM = 1       // Expects a single traditional RC PWM pulse
};

struct MainConfig {
  // --- Network / Identification ---
  uint8_t nodeId;             // Address of this module (e.g., 0x00 for Master, 0x10 for Trailer 1)

  // --- Routing: Inputs ---
  InputConfig sbusInputs[NUM_SBUS_CHANNELS];
  
  PpmInputMode ppmMode;       // Toggle between 8-channel PPM or single-channel PWM
  InputConfig ppmInputs[NUM_PPM_CHANNELS]; // If SINGLE_PWM, only ppmInputs[0] is evaluated
  LightInputChannel lightInputs;
  
  // --- Routing: Outputs (ESP32 Pins) ---
  LocalOutputConfig localOuts[NUM_LOCAL_OUTPUTS];
  
  // --- System Safety: Failsafe ---
  // Timeout in milliseconds without a valid PPM/PWM pulse before failsafe triggers
  uint16_t failsafeTimeoutMs; 
  
  // Which bits to force ON during a failsafe (e.g., BIT_HAZARD_LIGHT | BIT_BRAKE_LIGHT)
  uint32_t failsafeMask;      
};

// Global instance 
extern MainConfig activeMainConfig;