#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "../config/main-config.h"
#include "../config/light-mode.h"
#include "../config/global-effects-config.h"

class LocalOutputController {
private:
    Servo servos[NUM_LOCAL_OUTPUTS];
    
    // --- Fading Engine State ---
    uint16_t currentPwmValues[NUM_LOCAL_OUTPUTS]; 
    uint32_t lastFadeMillis[NUM_LOCAL_OUTPUTS];

    void resetOutput(uint8_t pin, uint8_t channel);

    // Helper: Calculates the exact target brightness based on priorities
    uint16_t calculateTargetPwm(const LocalOutputConfig& cfg, uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos);
    
    // Helper: Moves the current value towards the target over time
    void processFading(int index, const LocalOutputConfig& cfg, uint16_t targetPwm);

public:
    LocalOutputController();
    void begin();
    void update(uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos, uint16_t* servoStateArray);
};