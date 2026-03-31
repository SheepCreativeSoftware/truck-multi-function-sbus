#pragma once
#include <Arduino.h>
#include "../config/main-config.h"

#define PPM_HISTORY_SIZE 3
class PpmParser {
private:
    uint8_t inputPin;
    PpmInputMode mode; // Wird jetzt dynamisch gesetzt
    
    volatile uint32_t lastRiseTime;
    volatile uint16_t rawValues[NUM_PPM_CHANNELS];
    volatile uint8_t currentChannelCount;
    volatile uint32_t lastValidPulseTime;
    volatile uint32_t lastValidPulseStart;
    uint16_t smoothenValues[NUM_PPM_CHANNELS];
    uint16_t historyChannels[NUM_PPM_CHANNELS][PPM_HISTORY_SIZE];
    uint8_t historyIndex;
    uint32_t lastValidPacketTime;
    
    uint32_t activePpmMask;

    static void IRAM_ATTR handleInterrupt(void* arg);

public:
    // Konstruktor nur noch mit dem Pin
    PpmParser(uint8_t pin);
    
    void begin();
    
    // Neue Funktion zum Setzen oder Ändern des Modus zur Laufzeit
    void setMode(PpmInputMode newMode);
    
    void update(bool isLinkActive, uint16_t* servoStateArray);
    
    uint32_t getActiveMask();
    bool isPulsePresent();
    uint16_t getRawValue(uint8_t channel);
	uint16_t getNormalizedValue(uint8_t index);
    uint16_t getNormalizedSmoothValue(uint8_t index);
    void updateSmoothValue();
    uint16_t calculateMedian(uint8_t index);
};