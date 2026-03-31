#pragma once
#include <Arduino.h>

class HardwareSoftPWM {
private:
    static const uint8_t MAX_PINS = 14;
    static const uint8_t PWM_RESOLUTION = 100; // 0-100 steps
    
    static uint8_t activePins[MAX_PINS];
    static volatile uint8_t dutyCycles[MAX_PINS];
    static uint8_t pinCount;
    
    static volatile uint8_t currentStep;
    static hw_timer_t* timer;

    // The Interrupt Service Routine (ISR) MUST be static and loaded into IRAM
    static void IRAM_ATTR onTimer();

public:
    // Initializes the hardware timer
    static void begin();
    
    // Configures a pin for software PWM and adds it to the ISR loop
    static bool attach(uint8_t pin);
    
    // Updates the duty cycle (0 to 100) for a specific pin
    static void update(uint8_t pin, uint8_t duty);
	// Detaches all pins and safely turns them off
    static void reset();
};