#include "json-interface.h"

void JsonInterface::update(LocalOutputController& outputController) {
    if (Serial.available() == 0) return; 

    // 1024 bytes is enough for a chunky update block
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, Serial);

    if (error) {
        Serial.print("JSON Parse Failed: ");
        Serial.println(error.c_str());
        while(Serial.available()) Serial.read(); // Clear buffer
        return;
    }

    bool configChanged = false;

    // --- 1. OUTPUTS ---
    if (doc.containsKey("outputs")) {
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
        configChanged = true;
    }

    // --- 2. SBUS INPUTS ---
    if (doc.containsKey("sbus")) {
        JsonArray sbusInputs = doc["sbus"].as<JsonArray>();
        for (JsonObject inConfig : sbusInputs) {
            uint8_t ch = inConfig["ch"];
            if (ch < 16) {
                activeMainConfig.sbusInputs[ch].type = static_cast<InputType>(inConfig["type"].as<uint8_t>());
                activeMainConfig.sbusInputs[ch].targetMaskLow = inConfig["maskL"].as<uint32_t>();
                activeMainConfig.sbusInputs[ch].targetMaskMid = inConfig["maskM"].as<uint32_t>();
                activeMainConfig.sbusInputs[ch].targetMaskHigh = inConfig["maskH"].as<uint32_t>();
                activeMainConfig.sbusInputs[ch].thresholdLow = inConfig["thL"].as<uint16_t>();
                activeMainConfig.sbusInputs[ch].thresholdHigh = inConfig["thH"].as<uint16_t>();
                activeMainConfig.sbusInputs[ch].targetServoIndex = inConfig["srv"].as<uint8_t>();
            }
        }
        Serial.println("SBUS Inputs updated!");
        configChanged = true;
    }

    // --- 3. GLOBAL EFFECTS ---
    if (doc.containsKey("effects")) {
        JsonObject effConfig = doc["effects"];
        
        // Using | default_value to only update keys that are present in the JSON
        activeEffectsConfig.turnSignalFreq = effConfig["turnFreq"] | activeEffectsConfig.turnSignalFreq;
        activeEffectsConfig.starterDimFactor = effConfig["stDim"] | activeEffectsConfig.starterDimFactor;
        activeEffectsConfig.strobeFlashDuration = effConfig["strFlash"] | activeEffectsConfig.strobeFlashDuration;
        activeEffectsConfig.strobeShortPause = effConfig["strShort"] | activeEffectsConfig.strobeShortPause;
        activeEffectsConfig.strobeLongPause = effConfig["strLong"] | activeEffectsConfig.strobeLongPause;
        
        Serial.println("Effects updated!");
        configChanged = true;
    }

    // Optional: Trigger a save to NVS here if configChanged is true
    // if (configChanged) { saveConfigToNVS(); }
}