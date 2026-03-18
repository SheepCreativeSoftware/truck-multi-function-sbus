#pragma once
#include <Arduino.h>
#include <Preferences.h>
#include "../config/light-mode.h"
#include "../config/main-config.h"       // Contains your MainConfig struct
#include "../config/global-effects-config.h"    // Contains your GlobalEffectsConfig struct

// Declare the global instances so any file including this header can access them
extern MainConfig activeMainConfig;
extern GlobalEffectsConfig activeEffectsConfig;

class MemoryManager {
private:
  Preferences prefs;
  
  // NVS Namespace (Max 15 characters!)
  const char* NVS_NAMESPACE = "rc_truck"; 
  
  // Keys for our specific data blobs (Max 15 characters!)
  const char* KEY_MAIN_CFG = "main_cfg";
  const char* KEY_EFFECTS_CFG = "effects_cfg";

public:
  // Initializes the NVS partition
  void begin();

  // Loads data from flash into the active RAM structs, or sets defaults if empty
  void loadConfig();

  // Commits the current RAM structs to the flash memory
  void saveConfig();

  // Wipes the NVS namespace (useful for fresh setups or struct upgrades)
  void factoryReset();
};

// Declare the global manager instance
extern MemoryManager memoryManager;