#pragma once
#include <Arduino.h>

// --- LOCAL OUTPUT DEFINITIONS ---

enum class OutputMode : uint8_t {
  NONE = 0,           // Pin is disabled
  DIGITAL = 1,        // Simple HIGH/LOW (e.g., Relays, basic LEDs)
  PWM = 2,            // Dimmable output (ESP32 ledc hardware PWM)
  SERVO = 3           // 50Hz Servo or ESC control
};

struct LocalOutputConfig {
  uint8_t pin;             // The physical GPIO pin on the ESP32
  OutputMode mode;         // How should this pin be driven?
  
  uint32_t triggerMask;    // Which LightBits trigger this output? (e.g., BIT_LOW_BEAM)
  
  // Generic parameters depending on the selected OutputMode:
  // 
  // Mode DIGITAL:
  // - param1, param2, param3 are ignored.
  // 
  // Mode PWM:
  // Multi-purpose parameters.
  // Example for combined lights (Parking | Low | High):
  // param1 = Brightness for Parking (lowest priority)
  // param2 = Brightness for Low Beam
  // param3 = Brightness for High Beam (highest priority)
  //
  // Example for Beacon:
  // param1 = LED Index (e.g., 2nd LED in the circle)
  // param2 = Beacon Group ID (0 or 1, for independent beacons)
  //
  // Mode SERVO:
  // - param1 = Minimum pulse width in microseconds (e.g., 1000)
  // - param2 = Maximum pulse width in microseconds (e.g., 2000)
  // - param3 = Center/Neutral pulse width in microseconds (e.g., 1500)
  uint16_t param1;       
  uint16_t param2;       
  uint16_t param3;       
  // Global fade transition time for this specific pin.
  // 0 = Instant snap (good for strobes/blinkers)
  // 1-255 = Milliseconds to fade to the new target value (soft on/off)
  uint16_t fadeTime;
};