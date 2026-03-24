#include "local-outputs.h"

LocalOutputController::LocalOutputController() {}

void LocalOutputController::resetOutput(uint8_t pin, uint8_t channel) {
    ledPWM[channel].detachPin(pin);
    servos[channel].detach();
};

bool LocalOutputController::isSoftwarePWMOutput(uint8_t pin) {
    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        if (softwarePWMOutputs[i] == pin) {
            return true;
        }
    }
    return false; 
}

void LocalOutputController::begin() {
    // Allocate all 4 hardware timers for the ESP32 PWM engine to use for servos.
    // This prevents jitter and conflicts with standard LED PWM.
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);

    // Reset the list of outputs for re-evaluation
    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        softwarePWMOutputs[i] = 0;
    }

    uint8_t numberOfServoOutputs = 0;
    uint8_t numberOfHardwarePWMOutputs = 0;

    // Check how many Servos are in use
    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        LocalOutputConfig& cfg = activeMainConfig.localOutputs[i];
        if (cfg.mode == OutputMode::SERVO) {
            numberOfServoOutputs++;
        }
    }

    // Reserve one extra channel as two channels share the same timer
    if(numberOfServoOutputs % 2 != 0) {
        numberOfServoOutputs++;
    }

    // Define outputs that need to be Software PWM as Hardware PWM outputs are limited
    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        LocalOutputConfig& cfg = activeMainConfig.localOutputs[i];
        uint8_t currentUsedChannels = numberOfServoOutputs + numberOfHardwarePWMOutputs;

        if (cfg.mode == OutputMode::PWM && currentUsedChannels <= MAX_LEDC_CHANNELS) {
            numberOfHardwarePWMOutputs++;
        } else {
            softwarePWMOutputs[i] = cfg.pin;
        }
    }

    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        LocalOutputConfig& cfg = activeMainConfig.localOutputs[i];

        resetOutput(cfg.pin, i);
    }

    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        LocalOutputConfig& cfg = activeMainConfig.localOutputs[i];

        resetOutput(cfg.pin, i);

        switch (cfg.mode) {
            case OutputMode::NONE: 
                pinMode(cfg.pin, INPUT);
                break;
            case OutputMode::DIGITAL:
                pinMode(cfg.pin, OUTPUT);
                digitalWrite(cfg.pin, LOW); // Safe default state
                break;

            case OutputMode::PWM:
                if(isSoftwarePWMOutput(cfg.pin)) {
                    pinMode(cfg.pin, OUTPUT);
                    digitalWrite(cfg.pin, LOW); // Safe default state
                } else {
                    // 12-bit gives us 4096 steps of brightness for ultra-smooth fading
                    // Core 2.x requires a channel (0-15). We use 'i' as the channel.
                    // 1000 Hz is perfect for LEDs (no visible flicker, no camera banding)
                    ledPWM[i].attachPin(cfg.pin, 1000, 12);
                    // Turn it off by writing to the CHANNEL, not the pin
                    ledPWM[i].write(0);
                }
                break;
            case OutputMode::SERVO:
                // param2 = min pulse (e.g., 1000us)
                // param3 = max pulse (e.g., 2000us)
                servos[i].setPeriodHertz(50); // Standard RC servo frequency
                uint8_t GPIOpin = digitalPinToGPIONumber(cfg.pin);
                
                // Attach the pin and apply the custom pulse width limits from the config
                servos[i].attach(GPIOpin, cfg.param2, cfg.param3);
                
                // Optionally move to a safe center position immediately:
                // servos[i].writeMicroseconds(1500); 
                break;
        }
    }
}

void LocalOutputController::update(uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos, uint16_t* servoStateArray) {
    for (int i = 0; i < NUM_LOCAL_OUTPUTS; i++) {
        LocalOutputConfig& cfg = activeMainConfig.localOutputs[i];
        
        if (cfg.mode == OutputMode::NONE) continue;

        if (cfg.mode == OutputMode::PWM) {
            // 1. Brain: What is the base target brightness?
            uint16_t target = calculateTargetPwm(cfg, inputState, effectState, beacon1Pos, beacon2Pos);
            
            // 1.5. Modifier: Apply Starter Dimming if active and configured for this pin
            if ((inputState & BIT_STARTER_DIM) && (cfg.triggerMask & BIT_STARTER_DIM)) {
                // Reduce target brightness by the dim factor (assuming factor is 0-100%)
                // Cast to uint32_t prevents overflow during multiplication before division
                target = (uint16_t)(((uint32_t)target * activeEffectsConfig.starterDimFactor) / 100);
            }
            
            // 2. Muscle: Fade towards that target
            uint16_t actualValue = processFading(i, cfg, target);

            if(isSoftwarePWMOutput(cfg.pin)) {
                updatePWMMapping(i, actualValue);
                writeSoftwarePWMOutput(i, cfg.pin);
            } else {
                writeHardwarePWMOutput(i, actualValue);
            }
        }
        else if (cfg.mode == OutputMode::SERVO) {
            if (cfg.param1 != InputServoMapping::NONE && cfg.param1 <= InputServoMapping::SRV_REMOTE_6) {
                uint16_t normalizedValue = servoStateArray[cfg.param1 - 1];
                uint16_t pulseWidth = map(normalizedValue, 0, 2000, cfg.param2, cfg.param3);
                pulseWidth = constrain(pulseWidth, cfg.param2, cfg.param3);
                servos[i].writeMicroseconds(pulseWidth);
            }
        }
        else if (cfg.mode == OutputMode::DIGITAL) {
            // Digital ignores fading, just snaps based on the target > 0
            uint16_t target = calculateTargetPwm(cfg, inputState, effectState, beacon1Pos, beacon2Pos);
            digitalWrite(cfg.pin, target > 0 ? HIGH : LOW);
        }
    }
}

uint16_t LocalOutputController::calculateTargetPwm(const LocalOutputConfig& cfg, uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos) {
    // Create a clean evaluation state that ignores the starter dim bit completely.
    // This prevents the bit from accidentally triggering the fallback or early exit logic.
    uint32_t evalState = inputState & ~BIT_STARTER_DIM;
    uint32_t triggerMask = cfg.triggerMask & ~BIT_STARTER_DIM; // Also ignore the starter dim bit in the trigger mask for matching logic
    
    if (triggerMask & BIT_STATIC_OFF) return 0;
    if (triggerMask & BIT_STATIC_ON) return cfg.param1;

    // --- COMBO: US-Style Tail Light LEFT ---
    if (triggerMask == COMB_US_TAIL_L) {
        // 1. Regular turn signal has highest priority (overrides brake on this side)
        if (evalState & BIT_TURN_SIGNAL_L) {
            return (effectState & BIT_GLOBAL_TURN_L) ? cfg.param2 : 0;
        }
        
        // 2. Brake has second highest priority (overrides hazard)
        if (evalState & BIT_BRAKE_LIGHT) {
            return cfg.param2; 
        }

        // 3. Hazard lights (Priority 3: Only active if NOT braking)
        if (evalState & BIT_HAZARD_LIGHT) {
            return ((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R)) ? cfg.param2 : 0;
        }

        // 4. Parking (Lowest priority)
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1; 
        
        return 0; // Off
    }

    // --- COMBO: US-Style Tail Light RIGHT ---
    if (triggerMask == COMB_US_TAIL_R) {
        // Same logic, just for the right side
        if (evalState & BIT_TURN_SIGNAL_R) {
            return (effectState & BIT_GLOBAL_TURN_R) ? cfg.param2 : 0;
        }
        if (evalState & BIT_BRAKE_LIGHT) return cfg.param2;
        if (evalState & BIT_HAZARD_LIGHT) return ((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R)) ? cfg.param2 : 0;
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1;
        
        return 0;
    }

    // --- COMBO: US-Style Tail Light LEFT ---
    if (triggerMask == COMB_PARK_TURN_L) {
        // 1. Regular turn signal has highest priority (overrides brake on this side)
        if (evalState & BIT_TURN_SIGNAL_L) {
            return (effectState & BIT_GLOBAL_TURN_L) ? cfg.param2 : 0;
        }
        

        // 3. Hazard lights (Priority 2: Only active if NOT braking)
        if (evalState & BIT_HAZARD_LIGHT) {
            return ((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R)) ? cfg.param2 : 0;
        }

        // 4. Parking / Low beam (Lowest priority)
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1;
        
        return 0; // Off
    }

    // --- COMBO: US-Style Tail Light RIGHT ---
    if (triggerMask == COMB_PARK_TURN_R) {
        // Same logic, just for the right side
        if (evalState & BIT_TURN_SIGNAL_R) {
            return (effectState & BIT_GLOBAL_TURN_R) ? cfg.param2 : 0;
        }
        if (evalState & BIT_HAZARD_LIGHT) return ((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R)) ? cfg.param2 : 0;
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1;
        
        return 0;
    }

    // --- EFFECTS: Hazard Lights ---
    if (triggerMask & BIT_HAZARD_LIGHT) {
        if ((evalState & BIT_HAZARD_LIGHT) || ((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R))) {
            if((effectState & BIT_GLOBAL_TURN_L) && (effectState & BIT_GLOBAL_TURN_R)) {
                return cfg.param1;
            } else {
                return 0;
            }
        }
    }

    // --- EFFECTS: Turn Signal Left ---
    if (triggerMask & BIT_TURN_SIGNAL_L) {
        if ((evalState & BIT_TURN_SIGNAL_L) || (effectState & BIT_GLOBAL_TURN_L)) {
            if(effectState & BIT_GLOBAL_TURN_L) {
                return cfg.param1;
            } else {
                return 0;
            }
        }
    }

    // --- EFFECTS: Turn Signal Right ---
    if (triggerMask & BIT_TURN_SIGNAL_R) {
        if ((evalState & BIT_TURN_SIGNAL_R) || (effectState & BIT_GLOBAL_TURN_R)) {
            if(effectState & BIT_GLOBAL_TURN_R) {
                return cfg.param1;
            } else {
                return 0;
            }
        }
    }

    // ====================================================================
    // EARLY EXIT: If the state doesn't match the trigger mask AT ALL, target is 0
    if ((evalState & triggerMask) == 0) {
        return 0;
    }
    // ====================================================================

    // --- COMBO 1: Park & Brake Light ---
    if (triggerMask == COMB_PARK_AND_BRAKE) {
        if (evalState & BIT_BRAKE_LIGHT) return cfg.param2; 
        if (evalState & BIT_PARKING_LIGHT)  return cfg.param1; 
    }

    // --- COMBO 2: Headlights (Parking / Low / High) ---
    if (triggerMask == COMB_PARK_AND_FULL_BEAM) {
        if (evalState & BIT_HIGH_BEAM) return cfg.param3; 
        if (evalState & BIT_LOW_BEAM)  return cfg.param2; 
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1; 
    }

    // --- COMBO 3: Headlights (Parking / Low) ---
    if (triggerMask == COMB_PARK_AND_LOW_BEAM) {
        if (evalState & BIT_LOW_BEAM)  return cfg.param2; 
        if (evalState & BIT_PARKING_LIGHT) return cfg.param1; 
    }

    // --- COMBO 4: Headlights (Low / High) ---8_
    if (triggerMask == COMB_LOW_AND_HIGH_BEAM) {
        if (evalState & BIT_HIGH_BEAM)  return cfg.param2; 
        if (evalState & BIT_LOW_BEAM) return cfg.param1; 
    }

    // --- EFFECTS: Strobe Light ---
    if (triggerMask & BIT_STROBE_LIGHT) {
        if (evalState & BIT_STROBE_LIGHT) {
            if (effectState & BIT_GLOBAL_STROBE) {
                return cfg.param1;
            } else {
                return 0;
            }
        }
    }

    // --- EFFECTS: Beacon Light ---
    if ((triggerMask & BIT_BEACON_LIGHT) && (evalState & BIT_BEACON_LIGHT)) {
        if (cfg.param2 == 0 && cfg.param1 == beacon1Pos) {
            return cfg.param3;
        } else if (cfg.param2 == 1 && cfg.param1 == beacon2Pos) {
            return cfg.param3;
        } else {
            return 0;
        }
    }

    // --- FALLBACK: Standard Output ---
    return cfg.param1; 
}

uint16_t LocalOutputController::processFading(int index, const LocalOutputConfig& cfg, uint16_t targetPwm) {
    uint32_t currentMillis = millis();
    
    // If we are already at the target, do nothing
    if (currentPwmValues[index] == targetPwm) {
        lastFadeMillis[index] = currentMillis;
        return currentPwmValues[index];
    }

    // If fadeTime is 0, snap instantly (good for strobes)
    if (cfg.fadeTime == 0) {
        currentPwmValues[index] = targetPwm;

        return currentPwmValues[index];
    }

    // Calculate how much time has passed since the last frame
    uint32_t elapsed = currentMillis - lastFadeMillis[index];
    if (elapsed == 0) return currentPwmValues[index]; // Too fast, wait for next millisecond

    // Calculate step size. 
    // We want to cover 4095 steps (max 12-bit PWM) in 'fadeTime' milliseconds.
    // step = (elapsedTime * MaxResolution) / TotalFadeTime
    uint32_t step = (elapsed * 4095) / cfg.fadeTime;
    if (step == 0) step = 1; // Ensure it always moves by at least 1 unit

    // Move current value towards target
    if (currentPwmValues[index] < targetPwm) {
        currentPwmValues[index] += step;
        if (currentPwmValues[index] > targetPwm) currentPwmValues[index] = targetPwm; // Cap it
    } else {
        // Prevent underflow when subtracting
        if (currentPwmValues[index] > step) {
            currentPwmValues[index] -= step;
        } else {
            currentPwmValues[index] = 0;
        }
        if (currentPwmValues[index] < targetPwm) currentPwmValues[index] = targetPwm; // Cap it
    }

    lastFadeMillis[index] = currentMillis;
    
    // Apply the new physical brightness
    return currentPwmValues[index];
}

void LocalOutputController::writeHardwarePWMOutput(uint8_t pinIndex, uint16_t targetPwm) {
    ledPWM[pinIndex].write(targetPwm);
}

void LocalOutputController::updatePWMMapping(uint8_t pinIndex, uint16_t targetPwm) {
    if(lastPwmValues[pinIndex] != targetPwm) {
        lastPwmValues[pinIndex] = targetPwm;
        mappedDuties[pinIndex] = map(targetPwm, 0, 4095, 0, SOFTWARE_PWM_PERIODE);
    }
}

void LocalOutputController::writeSoftwarePWMOutput(uint8_t pinIndex, uint8_t pin) {
    if((micros() & SOFTWARE_PWM_MASK) < mappedDuties[pinIndex]) {
        digitalWrite(pin, HIGH);
    } else {
        digitalWrite(pin, LOW);
    }
}