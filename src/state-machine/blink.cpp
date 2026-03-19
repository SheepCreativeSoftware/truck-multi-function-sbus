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

#include "blink.h"

uint8_t Blink::blink(uint16_t blinkTimeMillis) {
	if((blinkOnTime == 0) || (blinkOnTime > millis())){ 	//Reset blinkOnTime on startup and on overflow.
		blinkOnTime = millis();
	}
		uint32_t blinkTime = millis() - blinkOnTime;
	if(blinkTime%blinkTimeMillis >= blinkTimeMillis/2){ 	//ON/OFF Interval at half of Time.
		return 0;
	} else {
		return 1;
	}

}

void Blink::resetBlink() {
	blinkOnTime = 0;
}
