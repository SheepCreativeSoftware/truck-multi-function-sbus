/************************************
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

#pragma once
#include <Arduino.h>

// The 16-bit master mask (The vocabulary of the bus)
enum LightBits : uint32_t {
  BIT_STATIC_OFF    = 0,
  // --- Standard Driving Lights ---
  BIT_PARKING_LIGHT = (1UL << 0),   // Value: 1
  BIT_LOW_BEAM      = (1UL << 1),   // Value: 2
  BIT_HIGH_BEAM     = (1UL << 2),   // Value: 4
  
  // --- Signal Lights ---
  BIT_TURN_SIGNAL_L = (1UL << 3),   // Value: 8
  BIT_TURN_SIGNAL_R = (1UL << 4),   // Value: 16
  BIT_HAZARD_LIGHT  = (1UL << 5),   // Value: 32
  BIT_BRAKE_LIGHT   = (1UL << 6),   // Value: 64
  BIT_REVERSE_LIGHT = (1UL << 7),   // Value: 128
  
  // --- Special Functions ---
  BIT_BEACON_LIGHT = (1UL << 8),   // Value: 256  (Rotating beacon)
  BIT_AUX_1         = (1UL << 9),   // Value: 512  (e.g., Work light)
  BIT_AUX_2         = (1UL << 10),  // Value: 1024 (e.g., Fog light)
  BIT_AUX_3         = (1UL << 11),  // Value: 2048 (e.g., Additional light)
  BIT_STARTER_DIM   = (1UL << 12),  // Value: 4096 (Starter dims lights)
  
  // --- System & Control Bits ---
  BIT_SHOWMODE      = (1UL << 13),  // Value: 8192 (Triggers show/sequence mode on slaves)
  BIT_STATIC_ON     = (1UL << 14),   // Value: 16384 (Virtual bit for always-on/marker lights)
  BIT_STROBE_LIGHT  = (1UL << 15),  // Value: 32768 (Warning strobes)
  BIT_FLASH_TO_PASS = (1UL << 16)   // Value: 65536 (Lichthupe / Optical horn sequence)
  // Available: Bits 17 to 31 (Over 2 billion possible combinations!)
};

enum CombinedLightStates: uint32_t {
  COMB_PARK_AND_FULL_BEAM = (BIT_PARKING_LIGHT | BIT_LOW_BEAM | BIT_HIGH_BEAM),
  COMB_PARK_AND_LOW_BEAM = (BIT_PARKING_LIGHT | BIT_LOW_BEAM),
  COMB_LOW_AND_HIGH_BEAM = (BIT_LOW_BEAM | BIT_HIGH_BEAM),
  COMB_PARK_AND_BRAKE = (BIT_PARKING_LIGHT | BIT_BRAKE_LIGHT),
  COMB_US_TAIL_L = (BIT_PARKING_LIGHT | BIT_LOW_BEAM | BIT_BRAKE_LIGHT | BIT_HAZARD_LIGHT | BIT_TURN_SIGNAL_L),
  COMB_US_TAIL_R = (BIT_PARKING_LIGHT | BIT_LOW_BEAM | BIT_BRAKE_LIGHT | BIT_HAZARD_LIGHT | BIT_TURN_SIGNAL_R),
};
