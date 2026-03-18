/************************************ 
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

#ifndef _TOOLS_H_
#define _TOOLS_H_
//Definition
#include "Arduino.h"


//Classes
class EdgeEvaluation {
		bool lastEdge;
    public:
		bool readEdge(bool input);
};

class Filter {
		int16_t lastValue;
		bool doneFilter;
    public:
		void init(int16_t initialValue);
		int16_t filterValue(int16_t input, int16_t filterFactor = 20, uint16_t filterTime = 100);
};

class Blink {
	private:
		uint32_t blinkOnTime = 0;
	public:
		uint8_t blink(uint16_t blinkTimeMillis);
		void resetBlink(); // Reset Blink after usage for next usage
};

#endif