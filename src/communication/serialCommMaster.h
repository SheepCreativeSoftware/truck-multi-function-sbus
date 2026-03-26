/************************************ 
 * Simple SerialBus Master Interface v0.0.1
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

#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>
#include "../config/main-config.h"
#include "../config/light-mode.h"
#include "../config/global-effects-config.h"

class SerialCommMaster {
public:
	void begin(HardwareSerial* serialPort, uint32_t baud, SerialConfig byteFormat, long timeout, long polling, uint8_t txEnablePin);

	uint16_t update(uint32_t inputState, uint32_t effectState,  uint16_t* servoStateArray);
	

private:
	// Constants
	static constexpr uint8_t BUFFER_SIZE = 64;
	static constexpr uint8_t MIN_BUFFER_SIZE = 4;
	static constexpr uint8_t BUFFER_EMPTY = 0;
	static constexpr uint8_t BROADCAST_ADDRESS = 0;
	static constexpr uint8_t FUNC_LIGHT_DATA = 1;
	static constexpr uint8_t FUNC_LIGHT_SERVO = 2;
	static constexpr uint8_t BIT_COUNT = 8;
	static constexpr uint16_t POLYNOMIAL = 0xA001;

	// State machine states
	enum State { IDLE, WAITING_FOR_TURNAROUND };

	// Member variables
	HardwareSerial* _serialPort;
	uint8_t _txEnablePin;
	uint16_t _errorCount;
	uint32_t _timeout;
	uint32_t _polling;
	uint16_t _frameDelay;
	uint32_t _delayStart;

	uint8_t _frame[BUFFER_SIZE];
	State _state;

	// Private methods
	void idle();
	void waitingForTurnaround();
	void constructPacket(uint32_t inputState, uint32_t effectState,  uint16_t* servoStateArray);
	uint16_t calculateCRC(uint8_t bufferSize);
	void sendPacket(uint8_t bufferSize);
};
