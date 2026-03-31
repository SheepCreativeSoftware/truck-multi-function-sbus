#pragma once
#include <Arduino.h>
#include <ESP32Servo.h>
#include "../config/main-config.h"
#include "../config/light-mode.h"
#include "../config/global-effects-config.h"
#include "hardware-soft-pwm.h"

#define MAX_LEDC_CHANNELS 8
// ~244Hz
#define SOFTWARE_PWM_PERIODE 4096UL
// The bitmask is always the period minus 1
#define SOFTWARE_PWM_MASK 4095UL

class LocalOutputController {
private:
    Servo servos[NUM_LOCAL_OUTPUTS];
    ESP32PWM ledPWM[NUM_LOCAL_OUTPUTS];
    
    // --- Fading Engine State ---
    uint16_t currentPwmValues[NUM_LOCAL_OUTPUTS]; 
    uint32_t lastFadeMillis[NUM_LOCAL_OUTPUTS];
    uint8_t softwarePWMOutputs[NUM_LOCAL_OUTPUTS];

    uint16_t xenonCurrentPwmValues[NUM_LOCAL_OUTPUTS]; 
    uint32_t xenonStartMillis[NUM_LOCAL_OUTPUTS];

    void resetOutput(uint8_t pin, uint8_t channel);

    bool isSoftwarePWMOutput(uint8_t pin);

    // Helper: Calculates the exact target brightness based on priorities
    uint16_t calculateTargetPwm(const LocalOutputConfig& cfg, uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos);
    uint16_t biXenonTargetPwm(int index, const LocalOutputConfig& cfg, uint32_t inputState, uint32_t effectState);
    
    // Helper: Moves the current value towards the target over time
    uint16_t processFading(int index, const LocalOutputConfig& cfg, uint16_t targetPwm);
    void writeHardwarePWMOutput(uint8_t pinIndex, uint16_t targetPwm);
    void updatePWMMapping(uint8_t pinIndex, uint16_t targetPwm);
    void writeSoftwarePWMOutput(uint8_t pinIndex, uint8_t pin);

public:
    LocalOutputController();
    void begin();
    void update(uint32_t inputState, uint32_t effectState, uint8_t beacon1Pos, uint8_t beacon2Pos, uint16_t* servoStateArray);
};