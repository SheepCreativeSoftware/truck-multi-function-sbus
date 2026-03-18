#include "esc-parser.h"

EscParser::EscParser() 
    : brakingActive(false), reverseActive(false), activeEscMask(0) {}

void EscParser::begin() {
    pinMode(activeMainConfig.lightInputs.brakeSignal, INPUT_PULLUP);
    pinMode(activeMainConfig.lightInputs.reverseSignal, INPUT_PULLUP);
}

void EscParser::update() {
    // Da das Signal reversed reinkommt (Active Low):
    // LOW (0V) -> Signal ist an (true)
    // HIGH (3.3V) -> Signal ist aus (false)
    brakingActive = (digitalRead(activeMainConfig.lightInputs.brakeSignal) == LOW);
    reverseActive = (digitalRead(activeMainConfig.lightInputs.reverseSignal) == LOW);

    uint32_t newMask = 0;

    if (brakingActive) {
        newMask |= BIT_BRAKE_LIGHT;
    }

    if (reverseActive) {
        newMask |= BIT_REVERSE_LIGHT;
    }

    activeEscMask = newMask;
}

uint32_t EscParser::getActiveMask() {
    return activeEscMask;
}