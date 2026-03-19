#include "esc-parser.h"

EscParser::EscParser(uint8_t brakePin, uint8_t reversePin) 
    : _brakePin(brakePin), _reversePin(reversePin),
    brakingActive(false), reverseActive(false), activeEscMask(0) {}

void EscParser::begin() {
    pinMode(_brakePin, INPUT_PULLUP);
    pinMode(_reversePin, INPUT_PULLUP);
}

void EscParser::update() {
    // Da das Signal reversed reinkommt (Active Low):
    // LOW (0V) -> Signal ist an (true)
    // HIGH (3.3V) -> Signal ist aus (false)
    brakingActive = (digitalRead(_brakePin) == LOW);
    reverseActive = (digitalRead(_reversePin) == LOW);

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