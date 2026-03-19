#pragma once
#include <Arduino.h>
#include <SerialIO.h>
#include "../config/main-config.h"

class SbusParser {
private:
    sbus receiver;
    rc_channels_t channelData;
    uint32_t activeSbusMask; 
    
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
};