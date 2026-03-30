#include "sbus-parser.h"

SbusParser::SbusParser(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin, bool invert)
    : receiver(serialPort, rxPin, txPin, invert), activeSbusMask(0) {
    // Initialize smoothenValues array with 1024 for all channels
    for (uint8_t i = 0; i < NUM_SBUS_CHANNELS; i++) {
        smoothenValues[i] = 1024;
        // Initialize historyChannels array with 1024 for all history entries
        for (uint8_t j = 0; j < SBUS_HISTORY_SIZE; j++) {
            historyChannels[i][j] = 1024;
        }
    }
    historyIndex = 0;
}

void SbusParser::begin() {
    receiver.begin();
}

void SbusParser::update(bool isLinkActive, uint16_t* servoStateArray) {
    receiver.processIncoming();

    // The main loop has determined the link is dead. Force failsafe mask.
    if (!isLinkActive) {
        activeSbusMask = activeMainConfig.failsafeMask;
        return; 
    }

    // Link is active, process normally
    receiver.getChannel(&channelData);
    updateSmoothValue();
    uint32_t newMask = 0;

    for (uint8_t i = 0; i < NUM_SBUS_CHANNELS; i++) {
        InputConfig& cfg = activeMainConfig.sbusInputs[i];
        uint16_t val = getSmoothValue(i);
        
        if (cfg.targetServoIndex != InputServoMapping::NONE && cfg.targetServoIndex <= InputServoMapping::SRV_REMOTE_6) {
            servoStateArray[cfg.targetServoIndex - 1] = val;
        }

        if (cfg.type == InputType::NONE) {
            continue;
        }

        if (cfg.type == InputType::SWITCH_3POS) {
            if (val < cfg.thresholdLow) {
                newMask |= cfg.targetMaskLow;
            } 
            else if (val >= cfg.thresholdLow && val <= cfg.thresholdHigh) {
                newMask |= cfg.targetMaskMid;
            } 
            else {
                newMask |= cfg.targetMaskHigh;
            }
        }
    }

    activeSbusMask = newMask;
}

uint32_t SbusParser::getActiveMask() {
    return activeSbusMask;
}

bool SbusParser::isSerialConnected() {
    return receiver.getSerialConnectionStatus();
}

bool SbusParser::isFailsafeActive() {
    return receiver.getFailsafe();
}

bool SbusParser::isFrameLost() {
    return receiver.getFramelost();
}

uint16_t SbusParser::getChannelValue(uint8_t index) {
    switch (index) {
        case 0:  return channelData.channel1;
        case 1:  return channelData.channel2;
        case 2:  return channelData.channel3;
        case 3:  return channelData.channel4;
        case 4:  return channelData.channel5;
        case 5:  return channelData.channel6;
        case 6:  return channelData.channel7;
        case 7:  return channelData.channel8;
        case 8:  return channelData.channel9;
        case 9:  return channelData.channel10;
        case 10: return channelData.channel11;
        case 11: return channelData.channel12;
        case 12: return channelData.channel13;
        case 13: return channelData.channel14;
        case 14: return channelData.channel15;
        case 15: return channelData.channel16;
        default: return 1024; 
    }
}

uint16_t SbusParser::getSmoothValue(uint8_t index) {
    
    return smoothenValues[index];
}

void SbusParser::updateSmoothValue() {
    uint32_t currentLastPacketTime = receiver.getLastValidPacketTime();
    if(currentLastPacketTime != lastValidPacketTime) {
        lastValidPacketTime = currentLastPacketTime;
        // New packet received, reset history
        for(uint8_t i = 0; i < NUM_SBUS_CHANNELS; i++) {
            historyChannels[i][historyIndex] = getChannelValue(i);

            smoothenValues[i] = calculateMedian(i);
        }

        historyIndex = (historyIndex + 1) % SBUS_HISTORY_SIZE;
    }
}

uint16_t SbusParser::calculateMedian(uint8_t index) {
    uint16_t sorted[SBUS_HISTORY_SIZE];
    memcpy(sorted, historyChannels[index], sizeof(uint16_t) * SBUS_HISTORY_SIZE);
    // Simple insertion sort
    for (uint8_t i = 1; i < SBUS_HISTORY_SIZE; i++) {
        uint16_t key = sorted[i];
        int8_t j = i - 1;
        while (j >= 0 && sorted[j] > key) {
            sorted[j + 1] = sorted[j];
            j--;
        }
        sorted[j + 1] = key;
    }
    return sorted[SBUS_HISTORY_SIZE / 2]; // Return median
}