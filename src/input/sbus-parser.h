#pragma once
#include <Arduino.h>
#include "./serialIO/SerialIO.h"
#include "../config/main-config.h"

#define SBUS_HISTORY_SIZE 5
class SbusParser {
private:
    sbus receiver;
    rc_channels_t channelData;
    uint32_t activeSbusMask; 
    uint16_t smoothenValues[NUM_SBUS_CHANNELS];
    uint16_t historyChannels[NUM_SBUS_CHANNELS][SBUS_HISTORY_SIZE];
    uint8_t historyIndex;
    uint32_t lastValidPacketTime;
    
    uint16_t getChannelValue(uint8_t index);

public:
    SbusParser(HardwareSerial* serialPort, int8_t rxPin, int8_t txPin, bool invert);
    
    void begin();
    
    // The main loop passes the combined connection status into the update function
    void update(bool isLinkActive, uint16_t* servoStateArray);
    
    uint32_t getActiveMask();
    
    // Expose hardware status for the main loop to evaluate
    bool isSerialConnected();
    bool isFailsafeActive();
    bool isFrameLost();
    uint16_t getSmoothValue(uint8_t index);
    void updateSmoothValue();
    uint16_t calculateMedian(uint8_t index);
};