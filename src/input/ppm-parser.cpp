#include "ppm-parser.h"

const uint16_t SYNC_PULSE_MIN = 850;
const uint16_t SYNC_PULSE_MAX = 980;

// Standardmäßig starten wir mit einem sicheren Modus
PpmParser::PpmParser(uint8_t pin) 
    : inputPin(pin), mode(PpmInputMode::MULTIPLEXED_8CH), lastRiseTime(0), currentChannelCount(0), activePpmMask(0) {
    for(int i=0; i < NUM_PPM_CHANNELS; i++) {rawValues[i] = 1500; smoothenValues[i]= 1500;}
}

void PpmParser::begin() {
    pinMode(inputPin, INPUT_PULLDOWN);
    attachInterruptArg(inputPin, PpmParser::handleInterrupt, this, CHANGE);
}

void PpmParser::setMode(PpmInputMode newMode) {
    mode = newMode;
    // Optional: Reset der Kanaldaten bei Moduswechsel
    currentChannelCount = 0;
}

void IRAM_ATTR PpmParser::handleInterrupt(void* arg) {
    PpmParser* instance = (PpmParser*)arg;
    uint32_t now = micros();

    uint32_t duration = now - instance->lastRiseTime;
    
    // Die ISR nutzt immer den aktuell gesetzten Modus der Instanz
    if (instance->mode == PpmInputMode::SINGLE_PWM) {
        if (duration >= 800 && duration <= 2200) {
            instance->rawValues[0] = (uint16_t)duration;
            instance->lastValidPulseTime = millis();
        }
    } 
    else {
        if (duration >= SYNC_PULSE_MIN && duration <= SYNC_PULSE_MAX) {
            instance->currentChannelCount = 0; 
            instance->lastValidPulseTime = millis();
        } 
        else if (duration >= 700 && duration <= 2200) {
            if (instance->currentChannelCount < NUM_PPM_CHANNELS) {
                uint8_t mirroredIndex = (NUM_PPM_CHANNELS - 1) - instance->currentChannelCount;
                instance->rawValues[mirroredIndex] = (uint16_t)duration;
                instance->currentChannelCount++;
                instance->lastValidPulseTime = millis();
            }
        }
    }
    instance->lastRiseTime = now;
}

void PpmParser::update(bool isLinkActive, uint16_t* servoStateArray) {
    if (!isLinkActive) {
        activePpmMask = 0;
        return;
    }

    uint32_t newMask = 0;
    uint8_t channelsToProcess = (mode == PpmInputMode::SINGLE_PWM) ? 1 : NUM_PPM_CHANNELS;

    for (uint8_t i = 0; i < channelsToProcess; i++) {
        InputConfig& cfg = activeMainConfig.ppmInputs[i];
        updateSmoothValue(i);
        
        if (cfg.targetServoIndex != InputServoMapping::NONE && cfg.targetServoIndex <= InputServoMapping::SRV_REMOTE_6) {
            uint16_t val = getNormalizedValue(i);
            servoStateArray[cfg.targetServoIndex - 1] = val;
        }

        if (cfg.type == InputType::NONE) {
            continue;
        }

        if (cfg.type == InputType::SWITCH_3POS) {
            uint16_t val = getNormalizedSmoothValue(i);
            if (val < cfg.thresholdLow) {
                newMask |= cfg.targetMaskLow;
            } else if (val >= cfg.thresholdLow && val <= cfg.thresholdHigh) {
                newMask |= cfg.targetMaskMid;
            } else {
                newMask |= cfg.targetMaskHigh;
            }
        }
    }
    activePpmMask = newMask;
}

bool PpmParser::isPulsePresent() {
    return (millis() - lastValidPulseTime < activeMainConfig.failsafeTimeoutMs);
}

uint32_t PpmParser::getActiveMask() { return activePpmMask; }
uint16_t PpmParser::getRawValue(uint8_t ch) { return rawValues[ch]; }

uint16_t PpmParser::getNormalizedValue(uint8_t index) {
    uint16_t raw = rawValues[index]; // Die us Werte (z.B. 930, 1500, 1977)
    
    // Mapping von Mikrosekunden auf unsere interne 0-2000 Skala
    // 1000us -> 0 | 1500us -> 1000 | 2000us -> 2000
    int32_t normalized = map(raw, 1000, 2000, 0, 2000);
    return (uint16_t)constrain(normalized, 0, 2000);
}

uint16_t PpmParser::getNormalizedSmoothValue(uint8_t index) {
    uint16_t smoothValue = smoothenValues[index]; // Die us Werte (z.B. 930, 1500, 1977)
    
    // Mapping von Mikrosekunden auf unsere interne 0-2000 Skala
    // 1000us -> 0 | 1500us -> 1000 | 2000us -> 2000
    int32_t normalized = map(smoothValue, 1000, 2000, 0, 2000);
    return (uint16_t)constrain(normalized, 0, 2000);
}

void PpmParser::updateSmoothValue(uint8_t index) {
    if(smoothenValues[index] != rawValues[index]) {
        smoothenValues[index] = (smoothenValues[index] * 3 + rawValues[index]) / 4;
    }
}