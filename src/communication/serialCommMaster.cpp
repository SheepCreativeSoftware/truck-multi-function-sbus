/************************************ 
 * Simple SerialBus Slave Interface v0.0.1
 * Date: 30.08.2022 | 21:52
 * <Truck Light and function module>
 * Copyright (C) 2020-2025 Marina Egner <hello@sheepcs.de>
 *
 * This program is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. 
 * If not, see <https://www.gnu.org/licenses/>.
 ************************************/
#include "serialCommMaster.h"

void SerialCommMaster::begin(
	HardwareSerial* serialPort,
	uint32_t baud,
	SerialConfig byteFormat,
	long timeout,
	long polling,
	uint8_t txEnablePin
) {
	_serialPort = serialPort;
	_timeout = timeout;
	_polling = polling;
	_txEnablePin = txEnablePin;

	(*_serialPort).begin(baud, byteFormat);
	pinMode(_txEnablePin, OUTPUT);
	digitalWrite(_txEnablePin, LOW);

	_errorCount = 0;
	_state = WAITING_FOR_TURNAROUND;
	_frameDelay = 10;
	_delayStart = 0;
}

uint16_t SerialCommMaster::update(uint32_t inputState, uint32_t effectState,  uint16_t* servoStateArray) {
	switch (_state) {
		case IDLE:
			constructPacket(inputState, effectState, servoStateArray);
			break;
		case WAITING_FOR_TURNAROUND:
			waitingForTurnaround();
			break;
	}
	return _errorCount;
}

void SerialCommMaster::constructPacket(
	uint32_t inputState, uint32_t effectState,  uint16_t* servoStateArray
) {
	// 1 byte function + 4 byte lightState + 2 byte effectState +
	// 9 byte ServoData (12bit) + 2 bytes CRC => 18 bytes total
	uint8_t frameSize = 18;
	_frame[0] = FUNC_LIGHT_DATA;

	_frame[1] = inputState & 0x00FF;
	_frame[2] = (inputState >> 8) & 0x00FF;
	_frame[3] = (inputState >> 16) & 0x00FF;
	_frame[4] = (inputState >> 24) & 0x00FF;

	_frame[5] = effectState & 0x00FF;
	_frame[6] = (effectState >> 8) & 0x00FF;
	_frame[7] = (servoStateArray[SRV_REMOTE_1] >> 4) & 0xFF; // First 8 bits of servo 1
	_frame[8] = ((servoStateArray[SRV_REMOTE_1] & 0x0F) << 4) | ((servoStateArray[SRV_REMOTE_2] >> 8) & 0x0F); // Last 4 bits of servo 1 and first 4 bits of servo 2
	_frame[9] = servoStateArray[SRV_REMOTE_2] & 0xFF; // Last 8 bits of servo 2
	_frame[10] = servoStateArray[SRV_REMOTE_3] >> 4; // First 8 bits of servo 3
	_frame[11] = ((servoStateArray[SRV_REMOTE_3] & 0x0F) << 4) | ((servoStateArray[SRV_REMOTE_4] >> 8) & 0x0F); // Last 4 bits of servo 3 and first 4 bits of servo 4
	_frame[12] = servoStateArray[SRV_REMOTE_4] & 0xFF; // Last 8 bits of servo 4
	_frame[13] = servoStateArray[SRV_REMOTE_5] >> 4; // First 8 bits of servo 5
	_frame[14] = ((servoStateArray[SRV_REMOTE_5] & 0x0F) << 4) | ((servoStateArray[SRV_REMOTE_6] >> 8) & 0x0F); // Last 4 bits of servo 5 and first 4 bits of servo 6
	_frame[15] = servoStateArray[SRV_REMOTE_6] & 0xFF; // Last 8 bits of servo 6

	uint16_t crc16 = calculateCRC(frameSize - 2);
	_frame[frameSize - 2] = crc16 >> 8; // Split crc into two bytes
	_frame[frameSize - 1] = crc16 & 0xFF;

	sendPacket(frameSize);
	_state = WAITING_FOR_TURNAROUND;
}

void SerialCommMaster::waitingForTurnaround() {
	if ((millis() - _delayStart) > _polling)
		_state = IDLE;
}

uint16_t SerialCommMaster::calculateCRC(uint8_t bufferSize) {
	uint16_t crc16 = 0xFFFF; // Load a 16–bit register with FFFF hex (all 1’s)
	for (uint8_t bufferPosition = 0; bufferPosition < bufferSize; bufferPosition++) {
		crc16 = crc16 ^ _frame[bufferPosition]; // XOR byte into least significant byte of crc
		for (uint8_t bitPosition = 1; bitPosition <= BIT_COUNT; bitPosition++) { // Loop over each bit
			if (crc16 & 0x0001) { // If the LSB is set
				crc16 >>= 1; // Shift right and XOR with polynomial
				crc16 ^= POLYNOMIAL;
			} else { // Shift only to the right if LSB is not set
				crc16 >>= 1;
			}
		}
	}
	return crc16; // The final content of the CRC register is the CRC value
}

void SerialCommMaster::sendPacket(uint8_t bufferSize) {
	digitalWrite(_txEnablePin, HIGH);

	for (unsigned char i = 0; i < bufferSize; i++) {
		(*_serialPort).write(_frame[i]);
	}
	(*_serialPort).flush();

	delayMicroseconds(_frameDelay);

	digitalWrite(_txEnablePin, LOW);

	_delayStart = millis(); // start the timeout delay
}