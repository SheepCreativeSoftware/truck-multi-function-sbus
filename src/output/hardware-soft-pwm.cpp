#include "hardware-soft-pwm.h"

// --- Static Member Initialization ---
uint8_t HardwareSoftPWM::activePins[HardwareSoftPWM::MAX_PINS];
volatile uint8_t HardwareSoftPWM::dutyCycles[HardwareSoftPWM::MAX_PINS];
uint8_t HardwareSoftPWM::pinCount = 0;
volatile uint8_t HardwareSoftPWM::currentStep = 0;
hw_timer_t* HardwareSoftPWM::timer = nullptr;

// --- Interrupt Service Routine ---
void IRAM_ATTR HardwareSoftPWM::onTimer() {
    currentStep++;
    if (currentStep >= PWM_RESOLUTION) {
        currentStep = 0;
    }

    // Toggle all registered pins based on their target duty cycle
    for (uint8_t i = 0; i < pinCount; i++) {
        if (dutyCycles[i] > currentStep) {
            digitalWrite(activePins[i], HIGH);
        } else {
            digitalWrite(activePins[i], LOW);
        }
    }
}

// --- Public Methods ---
void HardwareSoftPWM::begin() {
    if (timer == nullptr) {
        // Use Timer 0, prescaler 80 (80MHz / 80 = 1MHz -> 1 tick = 1 microsecond)
        timer = timerBegin(0, 80, true);
        
        // Attach the static ISR function
        timerAttachInterrupt(timer, &HardwareSoftPWM::onTimer, true);
        
        // Set alarm to trigger every 10 microseconds (100 steps * 10us = 1000Hz)
        timerAlarmWrite(timer, 10, true);
        
        // Start the timer
        timerAlarmEnable(timer);
    }
}

bool HardwareSoftPWM::attach(uint8_t pin) {
    if (pinCount < MAX_PINS) {
        // Register the pin
        activePins[pinCount] = pin;
        dutyCycles[pinCount] = 0; // Default to OFF
        
        // Configure hardware pin
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        
        pinCount++;
        return true; // Successfully attached
    }
    return false; // Array is full
}

void HardwareSoftPWM::update(uint8_t pin, uint8_t duty) {
    // Constrain input to avoid buffer overflows or logical errors
    if (duty > PWM_RESOLUTION) {
        duty = PWM_RESOLUTION;
    }

    // Search for the pin and update its duty cycle
    for (uint8_t i = 0; i < pinCount; i++) {
        if (activePins[i] == pin) {
            dutyCycles[i] = duty;
            break;
        }
    }
}

void HardwareSoftPWM::reset() {
    // 1. Safely turn off all currently active pins to prevent stuck states
    for (uint8_t i = 0; i < pinCount; i++) {
        digitalWrite(activePins[i], LOW);
        
        // Optional but clean: clear the arrays
        activePins[i] = 0;
        dutyCycles[i] = 0;
    }

    // 2. Reset the counters. 
    // The ISR will now skip its execution loop since pinCount is 0.
    pinCount = 0;
    currentStep = 0;
}