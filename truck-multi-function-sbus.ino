/************************************ 
 * truck-multi-function-sbus v2.0.0
 * Date: 20.08.2025
 * <Truck Light and function module>
 * Copyright (C) 2020-2026 Marina Egner <hello@sheepcs.de>
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

/************************************
 * Include Module and Library Files
 ************************************/
#include <Arduino.h>
#include "src/memory/memory-manager.h"
#include "src/input/sbus-parser.h"
#include "src/input/ppm-parser.h"
#include "src/input/esc-parser.h"

// Initialize the parser using hardware Serial1, RX on pin 16, TX disabled (-1)
SbusParser sbusInput(&Serial1, D3, -1, false);
PpmParser  ppmInput(D4);
EscParser  escInput;

void setup() {
  Serial.begin(115200);
  
  // Initialize storage and load configs into RAM
  memoryManager.begin();
  memoryManager.loadConfig();

  ppmInput.setMode(activeMainConfig.ppmMode);

  sbusInput.begin();
  ppmInput.begin();
  escInput.begin();
  
  Serial.println("System Booted. Configurations loaded successfully.");
}

void loop() {
  bool systemConnected = (sbusInput.isSerialConnected() && ppmInput.isPulsePresent());

  sbusInput.update(systemConnected);
  ppmInput.update(systemConnected);
  escInput.update();

  uint32_t globalState = sbusInput.getActiveMask() | 
                          ppmInput.getActiveMask() | 
                          escInput.getActiveMask();
}