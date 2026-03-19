#pragma once
#include <Arduino.h>
#include "../config/main-config.h"
#include "../config/light-mode.h"

class EscParser {
private:
    bool brakingActive;
    bool reverseActive;
    uint8_t _brakePin;
    uint8_t _reversePin;

    uint32_t activeEscMask;

public:
    EscParser(uint8_t brakePin, uint8_t reversePin);
    
    void begin();
    
    // Liest die Hardware-Pins und berechnet die Maske basierend auf der Config
    void update();
    
    uint32_t getActiveMask();
    
    bool isBraking() { return brakingActive; }
    bool isReverse() { return reverseActive; }
};