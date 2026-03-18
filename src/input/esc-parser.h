#pragma once
#include <Arduino.h>
#include "../config/main-config.h"
#include "../config/light-mode.h"

class EscParser {
private:
    bool brakingActive;
    bool reverseActive;

    uint32_t activeEscMask;

public:
    EscParser();
    
    void begin();
    
    // Liest die Hardware-Pins und berechnet die Maske basierend auf der Config
    void update();
    
    uint32_t getActiveMask();
    
    bool isBraking() { return brakingActive; }
    bool isReverse() { return reverseActive; }
};