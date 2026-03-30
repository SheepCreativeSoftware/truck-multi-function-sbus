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
#include "src/state-machine/global-effects.h"
#include "src/output/local-outputs.h"
#include "src/config-interface/json-interface.h"
#include "src/communication/serialCommMaster.h"

// Initialize the parser using hardware Serial1, RX on pin 16, TX disabled (-1)
SbusParser sbusInput(&Serial1, D3, -1, false);
PpmParser  ppmInput(D4);
EscParser  escInput(A2, A4);
GlobalEffects globalEffects;
LocalOutputController localOutputController;
JsonInterface jsonUi;
SerialCommMaster serialCommMaster;
// In main.cpp
// Index 0-5: Local Master Board Servos
// Index 6-11: Remote RS485 Bus Servos
// Initialized to 1000 (which represents the 1500µs center position on our 0-2000 scale)
uint16_t globalServoState[12] = {1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000, 1000};

void setup() {
  Serial.begin(115200);
  
  // Initialize storage and load configs into RAM
  memoryManager.begin();
  memoryManager.loadConfig();

  ppmInput.setMode(activeMainConfig.ppmMode);

  sbusInput.begin();
  ppmInput.begin();
  escInput.begin();

  localOutputController.begin();

  serialCommMaster.begin(&Serial0, 19200, SERIAL_8N1, 1000, 30, D2);

  uint32_t initialProccessingTime = millis();
  while (millis() - initialProccessingTime < 1000) {
    sbusInput.update(false, globalServoState);
  }
}


void loop() {
  jsonUi.update(localOutputController, memoryManager);
  bool systemConnected = (sbusInput.isSerialConnected() && !sbusInput.isFailsafeActive() && ppmInput.isPulsePresent());

  sbusInput.update(systemConnected, globalServoState);
  ppmInput.update(systemConnected, globalServoState);
  escInput.update();

  uint32_t globalState = sbusInput.getActiveMask() | 
                          ppmInput.getActiveMask() | 
                          escInput.getActiveMask();
  

  globalEffects.update(globalState);

  uint16_t effectState = globalEffects.getActiveEffectMask();
  uint8_t beacon1pos = globalEffects.getBeaconPosition(1);
  uint8_t beacon2pos = globalEffects.getBeaconPosition(2);

  serialCommMaster.update(globalState, effectState, globalServoState);
  
  localOutputController.update(globalState, effectState, beacon1pos, beacon2pos, globalServoState);
}