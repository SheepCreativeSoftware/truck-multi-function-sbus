#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../config/main-config.h"
#include "../config/global-effects-config.h"
#include "../output/local-outputs.h"
#include "../memory/memory-manager.h"

class JsonInterface {
public:
    // Pass the output controller so we can re-initialize pins if the config changes
    void update(LocalOutputController& outputController, MemoryManager& memoryManager);
};