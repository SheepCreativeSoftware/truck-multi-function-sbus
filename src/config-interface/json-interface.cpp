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

    // --- Main Config ---
    if(doc["main"].is<JsonObject>()) {
        JsonObject genConfig = doc["main"];
        activeMainConfig.failsafeMask = genConfig["fsMask"].as<uint32_t>() | activeMainConfig.failsafeMask;
        activeMainConfig.failsafeTimeoutMs = genConfig["fsTimeout"].as<uint16_t>() | activeMainConfig.failsafeTimeoutMs;
        activeMainConfig.ppmMode = static_cast<PpmInputMode>(genConfig["ppmMode"].as<uint8_t>() | static_cast<uint8_t>(activeMainConfig.ppmMode));
        Serial.println("{ \"status\": \"Main config updated\" }");
    }

    // --- 1. OUTPUTS ---
    if (doc["outputs"].is<JsonArray>()) {
        JsonArray outputs = doc["outputs"].as<JsonArray>();
        size_t index = 0;
        for (JsonObject outConfig : outputs) {
            if(index < NUM_LOCAL_OUTPUTS && index < outputs.size()) {
                activeMainConfig.localOutputs[index].pin = outConfig["pin"].as<uint8_t>();
                activeMainConfig.localOutputs[index].mode = static_cast<OutputMode>(outConfig["mode"].as<uint8_t>());
                activeMainConfig.localOutputs[index].triggerMask = outConfig["mask"].as<uint32_t>();
                activeMainConfig.localOutputs[index].param1 = outConfig["p1"].as<uint16_t>();
                activeMainConfig.localOutputs[index].param2 = outConfig["p2"].as<uint16_t>();
                activeMainConfig.localOutputs[index].param3 = outConfig["p3"].as<uint16_t>();
                activeMainConfig.localOutputs[index].fadeTime = outConfig["fade"].as<uint16_t>();
            }
            index++;
        }
        Serial.println("{ \"status\": \"Outputs updated\" }");
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
        Serial.println("{ \"status\": \"SBUS Inputs updated\" }");
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
        Serial.println("{ \"status\": \"PPM Inputs updated\" }");
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
        activeEffectsConfig.xenonFlashDuration = effConfig["xenFlash"] | activeEffectsConfig.xenonFlashDuration;
        activeEffectsConfig.xenonFadeDuration = effConfig["xenFade"] | activeEffectsConfig.xenonFadeDuration;
        activeEffectsConfig.xenonLowBeamStartPwm = effConfig["xenLowPwm"] | activeEffectsConfig.xenonLowBeamStartPwm;
        
        Serial.println("{ \"status\": \"effects updated\" }");
    }

    // Optional: Trigger a save to NVS here if configChanged is true
    if (doc["save"].is<JsonString>()) {
        JsonString save = doc["save"];
        if(save == "true") {
            memoryManager.saveConfig();
            Serial.println("{ \"status\": \"config saved\" }");
        }
    }

    if (doc["factory"].is<JsonString>()) {
        JsonString factory = doc["factory"];
        if(factory == "true") {
            memoryManager.factoryReset();
            Serial.println("{ \"status\": \"factory reset\" }");
        }
    }

    if (doc["status"].is<JsonString>()) {
        Serial.println("{ \"status\": \"ok\", \"version\": \"2.0.0\", \"model\": \"MainESP32\" }");
    }
}