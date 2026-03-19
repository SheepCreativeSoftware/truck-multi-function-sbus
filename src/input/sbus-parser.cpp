#include "sbus-parser.h"

SbusParser::SbusParser(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin, bool invert)
    : receiver(serialPort, rxPin, txPin, invert), activeSbusMask(0) {}

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
    uint32_t newMask = 0;

    for (uint8_t i = 0; i < NUM_SBUS_CHANNELS; i++) {
        InputConfig& cfg = activeMainConfig.sbusInputs[i];

        if (cfg.type == InputType::NONE) {
            continue;
        }

        uint16_t val = getChannelValue(i);
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
        else if (cfg.type == InputType::PROPORTIONAL) {
            if (cfg.targetServoIndex < 12) {
                servoStateArray[cfg.targetServoIndex] = val;
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
        default: return 1500; 
    }
}