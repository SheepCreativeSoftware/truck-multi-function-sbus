#include "memory-manager.h"

// Instantiate the actual memory blocks in RAM
MainConfig activeMainConfig;
GlobalEffectsConfig activeEffectsConfig;
MemoryManager memoryManager;

void MemoryManager::begin() {
  // Open the namespace in read/write mode (false)
  prefs.begin(NVS_NAMESPACE, false); 
}

void MemoryManager::loadConfig() {
  // --- 1. Load MainConfig ---
  size_t mainSize = prefs.getBytesLength(KEY_MAIN_CFG);
  if (mainSize == sizeof(MainConfig)) {
    prefs.getBytes(KEY_MAIN_CFG, &activeMainConfig, sizeof(MainConfig));
  } else {
    // Memory is empty or struct size changed.
    // Initialize critical defaults here to prevent undefined behavior.
    activeMainConfig.nodeId = 0; // 0 = Master Controller
    activeMainConfig.ppmMode = PpmInputMode::MULTIPLEXED_8CH;
    activeMainConfig.failsafeTimeoutMs = 2000;
    activeMainConfig.failsafeMask = BIT_HAZARD_LIGHT; // Or e.g., BIT_HAZARD_LIGHT
  }

  // --- 2. Load GlobalEffectsConfig ---
  size_t effectsSize = prefs.getBytesLength(KEY_EFFECTS_CFG);
  if (effectsSize == sizeof(GlobalEffectsConfig)) {
    prefs.getBytes(KEY_EFFECTS_CFG, &activeEffectsConfig, sizeof(GlobalEffectsConfig));
  } else {
    // Apply safe defaults for the very first boot
    activeEffectsConfig.turnSignalFreq = 500;
    activeEffectsConfig.strobeFlashDuration = 40;
    activeEffectsConfig.strobeShortPause = 60;
    activeEffectsConfig.strobeLongPause = 400;
    activeEffectsConfig.flashToPassFreq = 100;
    activeEffectsConfig.beacon1Speed = 2000;
    activeEffectsConfig.beacon1MaxLeds = 4;
    activeEffectsConfig.beacon2Speed = 2000;
    activeEffectsConfig.beacon2MaxLeds = 4;
    activeEffectsConfig.starterDimFactor = 50;
    activeEffectsConfig.corneringLightOffDelay = 5000;
  }
}

void MemoryManager::saveConfig() {
  prefs.putBytes(KEY_MAIN_CFG, &activeMainConfig, sizeof(MainConfig));
  prefs.putBytes(KEY_EFFECTS_CFG, &activeEffectsConfig, sizeof(GlobalEffectsConfig));
}

void MemoryManager::factoryReset() {
  prefs.clear(); // Deletes all keys in the current namespace
  loadConfig();  // Reloads the safe defaults into RAM
}