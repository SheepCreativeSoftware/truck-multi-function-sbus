#include "json-interface.h"

void JsonInterface::update(LocalOutputController& outputController, MemoryManager& memoryManager) {
    if (Serial.available() == 0) return; 

    // 1024 bytes is enough for a chunky update block
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, Serial);

    if (error) {
        Serial.print("JSON Parse Failed: ");
        Serial.println(error.c_str());
        while(Serial.available()) Serial.read(); // Clear buffer
        return;
    }

    // --- 1. OUTPUTS ---
    if (doc["outputs"].is<JsonArray>()) {
        JsonArray outputs = doc["outputs"].as<JsonArray>();
        for (JsonObject outConfig : outputs) {
            uint8_t targetPin = outConfig["pin"];
            for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
                if (activeMainConfig.localOutputs[i].pin == targetPin || activeMainConfig.localOutputs[i].mode == OutputMode::NONE) {
                    activeMainConfig.localOutputs[i].pin = targetPin;
                    activeMainConfig.localOutputs[i].mode = static_cast<OutputMode>(outConfig["mode"].as<uint8_t>());
                    activeMainConfig.localOutputs[i].triggerMask = outConfig["mask"].as<uint32_t>();
                    activeMainConfig.localOutputs[i].param1 = outConfig["p1"].as<uint16_t>();
                    activeMainConfig.localOutputs[i].param2 = outConfig["p2"].as<uint16_t>();
                    activeMainConfig.localOutputs[i].param3 = outConfig["p3"].as<uint16_t>();
                    activeMainConfig.localOutputs[i].fadeTime = outConfig["fade"].as<uint16_t>();
                    break; 
                }
            }
        }
        Serial.println("Outputs updated!");
        outputController.begin(); // Re-init hardware pins
    }

    // --- 2. SBUS INPUTS ---
    if (doc["sbus"].is<JsonArray>()) {
        JsonArray sbusInputs = doc["sbus"].as<JsonArray>();
        for (JsonObject inConfig : sbusInputs) {
            uint8_t ch = inConfig["ch"];
            uint8_t channel = ch - 1;
            if (channel < 16) {
                activeMainConfig.sbusInputs[channel].type = static_cast<InputType>(inConfig["type"].as<uint8_t>());
                activeMainConfig.sbusInputs[channel].targetMaskLow = inConfig["maskL"].as<uint32_t>();
                activeMainConfig.sbusInputs[channel].targetMaskMid = inConfig["maskM"].as<uint32_t>();
                activeMainConfig.sbusInputs[channel].targetMaskHigh = inConfig["maskH"].as<uint32_t>();
                activeMainConfig.sbusInputs[channel].thresholdLow = inConfig["thL"].as<uint16_t>();
                activeMainConfig.sbusInputs[channel].thresholdHigh = inConfig["thH"].as<uint16_t>();
                activeMainConfig.sbusInputs[channel].targetServoIndex = inConfig["srv"].as<InputServoMapping>();
            }
        }
        Serial.println("SBUS Inputs updated!");
    }

    // --- 2.1. PPM INPUTS ---
    if (doc["ppmIn"].is<JsonArray>()) {
        JsonArray ppmInputs = doc["ppmIn"].as<JsonArray>();
        for (JsonObject inConfig : ppmInputs) {
            uint8_t ch = inConfig["ch"];
            uint8_t channel = ch - 1;
            if (channel < 8) {
                activeMainConfig.ppmInputs[channel].type = static_cast<InputType>(inConfig["type"].as<uint8_t>());
                activeMainConfig.ppmInputs[channel].targetMaskLow = inConfig["maskL"].as<uint32_t>();
                activeMainConfig.ppmInputs[channel].targetMaskMid = inConfig["maskM"].as<uint32_t>();
                activeMainConfig.ppmInputs[channel].targetMaskHigh = inConfig["maskH"].as<uint32_t>();
                activeMainConfig.ppmInputs[channel].thresholdLow = inConfig["thL"].as<uint16_t>();
                activeMainConfig.ppmInputs[channel].thresholdHigh = inConfig["thH"].as<uint16_t>();
                activeMainConfig.ppmInputs[channel].targetServoIndex = inConfig["srv"].as<InputServoMapping>();
            }
        }
        Serial.println("PPM Inputs updated!");
    }

    // --- 3. GLOBAL EFFECTS ---
    if (doc["effects"].is<JsonObject>()) {
        JsonObject effConfig = doc["effects"];
        
        // Using | default_value to only update keys that are present in the JSON
        activeEffectsConfig.turnSignalFreq = effConfig["turnFreq"] | activeEffectsConfig.turnSignalFreq;
        activeEffectsConfig.starterDimFactor = effConfig["stDim"] | activeEffectsConfig.starterDimFactor;
        activeEffectsConfig.strobeFlashDuration = effConfig["strFlash"] | activeEffectsConfig.strobeFlashDuration;
        activeEffectsConfig.strobeShortPause = effConfig["strShort"] | activeEffectsConfig.strobeShortPause;
        activeEffectsConfig.strobeLongPause = effConfig["strLong"] | activeEffectsConfig.strobeLongPause;
        activeEffectsConfig.beacon1Speed = effConfig["bcn1Spd"] | activeEffectsConfig.beacon1Speed;
        activeEffectsConfig.beacon2Speed = effConfig["bcn2Spd"] | activeEffectsConfig.beacon2Speed;
        activeEffectsConfig.beacon1MaxLeds = effConfig["bcn1Max"] | activeEffectsConfig.beacon1MaxLeds;
        activeEffectsConfig.beacon2MaxLeds = effConfig["bcn2Max"] | activeEffectsConfig.beacon2MaxLeds;
        activeEffectsConfig.flashToPassFreq = effConfig["ftpFreq"] | activeEffectsConfig.flashToPassFreq;
        activeEffectsConfig.corneringLightOffDelay = effConfig["cornerOff"] | activeEffectsConfig.corneringLightOffDelay;
        
        Serial.println("Effects updated!");
    }

    // Optional: Trigger a save to NVS here if configChanged is true
    if (doc["save"].is<JsonString>()) {
        JsonString save = doc["save"];
        if(save == "true") {
            memoryManager.saveConfig();
            Serial.println("Config Saved!");
        }
    }

    if (doc["factory"].is<JsonString>()) {
        JsonString factory = doc["factory"];
        if(factory == "true") {
            memoryManager.factoryReset();
            Serial.println("Factory Reset!");
        }
    }
}