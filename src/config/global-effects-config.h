#pragma once
#include <Arduino.h>

enum GlobalOutputEffects: uint16_t {
	BIT_GLOBAL_TURN_L = (1U << 0),
	BIT_GLOBAL_TURN_R = (1U << 1),
	BIT_GLOBAL_STROBE = (1U << 2),
};

struct GlobalEffectsConfig {
  // --- Timings in milliseconds ---
  
  // Turn signals & Hazards (usually identical speed, but distinct logic)
  uint16_t turnSignalFreq;   // e.g., 500ms ON / 500ms OFF

  // Strobes (Double Flash Sequence / Doppelblitz)
  // Pattern: ON -> shortPause -> ON -> longPause -> (repeat)
  uint16_t strobeFlashDuration; // e.g., 40  (Time the LED stays ON)
  uint16_t strobeShortPause;    // e.g., 60  (Pause between the two rapid flashes)
  uint16_t strobeLongPause;     // e.g., 400 (Pause before the next double flash starts)
  
  // Flash to pass (Lichthupe)
  uint16_t flashToPassFreq;  // e.g., 100ms ON / 100ms OFF
  
  // Rotating beacon (Rundumleuchte)
  uint16_t beacon1Speed;      // Time between LED transition steps
  uint16_t beacon2Speed;      // Time between LED transition steps
  uint8_t beacon1MaxLeds;     // Amount of virtual LEDs in the beacon circle
  uint8_t beacon2MaxLeds;     // Amount of virtual LEDs in the beacon circle
  
  // --- Specific settings ---
  uint8_t starterDimFactor;  // Dimming percentage (e.g., 50%) during motor start
};

extern GlobalEffectsConfig activeEffectsConfig;