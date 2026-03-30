/*!
 * @file sbus.cpp
 * @brief Source file for the SBus implementations
 * @author Witty-Wizard
 */
#include "sbus.h"

sbus::sbus(Stream *rxPort, int rxPin, int txPin, bool inverted)
    : SerialIO(rxPort, rxPin, txPin, inverted) {}

void sbus::begin() {

// Initialize the serial port
#if defined(ARDUINO_ARCH_ESP32)
  HardwareSerial *serialPort = (HardwareSerial *)_rxPort;
  serialPort->begin(SBUS_BAUDRATE, SERIAL_8E2, _rxPin, _txPin, _inverted);
#elif defined(ARDUINO_ARCH_RP2040)
  SerialUART *serialPort = (SerialUART *)_rxPort;
  serialPort->setPinout(_txPin, _rxPin);
  serialPort->setInvertRX(_inverted);
  serialPort->setInvertTX(_inverted);
  serialPort->begin(SBUS_BAUDRATE, SERIAL_8E2);
#else
#warning "Unsupported hardware platform."
#endif
}

void sbus::processIncoming() {
  while (_rxPort->available()) {
    uint32_t now = micros();
    uint8_t incomingByte = _rxPort->read();
    
    // If there was a gap, we MUST be at the start of a frame
    if (now - lastByteMicros > SBUS_GAP_THRESHOLD) {
      bufferIdx = 0; 
    }
    lastByteMicros = now;

    if (bufferIdx < 25) {
      _rxData[bufferIdx++] = incomingByte;
    }

    if (bufferIdx == 25) {
      if (_rxData[0] == HEADER_SBUS &&
        _rxData[SBUS_MAX_PACKET_SIZE - 1] == FOOTER_SBUS) {

        memcpy(&_channelData, _rxData, sizeof(_channelData));

        _lastValidPacketTime = millis();
        _connectionTimeout = false;
      } else {
        // Potential "False Sync" - discard and wait for next gap
        bufferIdx = 0;
      }
    }
  }

  if (millis() - _lastValidPacketTime > SBUS_TIMEOUT) {
    _connectionTimeout = true;
  }
}

void sbus::getChannel(rc_channels_t *channelData) {
  memcpy(channelData, (uint8_t *)&_channelData + 1, sizeof(rc_channels_t));
}

bool sbus::getFailsafe() {
  return _channelData.failsafe;
}

bool sbus::getFramelost() {
  return _channelData.framelost;
}

bool sbus::getChannel17() {
  return _channelData.channel17;
}

bool sbus::getChannel18() {
  return _channelData.channel18;
}

bool sbus::getSerialConnectionStatus() {
  return !_connectionTimeout;
}

uint32_t sbus::getLastValidPacketTime() {
    return _lastValidPacketTime;
  }