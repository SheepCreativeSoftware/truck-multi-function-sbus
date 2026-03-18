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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "vehicleConfig.h"

/************************************
 * Configuration Programm
 ************************************/

VehicleConfig vehicleConfig = {
	.generalConfig = {
		.countryOption = CountryOption::US,
		.debugLevel = DebugLevel::STATUS_ONLY,
		.statusLightPin = 13
	},
	.ppmConfig = {
		.pinChannel1 = 2,
		.pinChannel2 = 3,
		.pinSoundChannel = 7
	},
	.generalLightConfig = {
		.fadeOnTime = 200,
		.fadeOffTime = 200,
		.starterDimmingFactor = 5,
		.starterDimmingMultiplier = 2
	},
	.serialConfig = {
		.isEnabled = true,
		.outTxEnablePin = 4,
		.baudRate = 19200,
		.byteFormat = SERIAL_8N1,
		.timeout = 1000,
		.pollingInterval = 20,
		.protocolVersion = ProtocolVersion::V2
	},
	.lightInputChannel = {
		.reverseSignal = A4,
		.brakeSignal = A5
	},
	.rearLeftTurnLight = {
		.outputPin = 5,
		.primaryOnBrightness = 255,
		// Parking when combined
		.secondaryOnBrightness = 15,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200,
	},
	.rearRightTurnLight = {
		.outputPin = 6,
		.primaryOnBrightness = 255,
		// Parking when combined
		.secondaryOnBrightness = 15,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.frontLeftTurnLight = {
		.outputPin = 8,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.frontRightTurnLight = {
		.outputPin = 9,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.lowBeamLight = {
		.outputPin = 10,
		// Low Beam or High beam when combined
		.primaryOnBrightness = 255,
		// Low Beam when combined with High Beam
		.secondaryOnBrightness = 100,
		// Parking when combined with Low Beam
		.tertiaryOnBrightness = 5,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.highBeamLight = {
		.outputPin = 11,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.parkingLight = {
		.outputPin = A3,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.fogLight = {
		.outputPin = A2,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.reverseLight = {
		.outputPin = A1,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.brakeLight = {
		.outputPin = 12,
		.primaryOnBrightness = 255,
		// Parking when combined
		.secondaryOnBrightness = 15,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.auxLight = {
		.outputPin = A0,
		.primaryOnBrightness = 255,
		.secondaryOnBrightness = 0,
		.tertiaryOnBrightness = 0,
		.offBrightness = 0,
		.fadeOnTime = 200,
		.fadeOffTime = 200
	},
	.lowBeamConfig = {
		.isParkingLight = true,
		.isHighBeam = true
	},
	.highBeamConfig = {
		.flashFrequency = 800
	},
	.turnSignalConfig = {
		.flashFrequency = 1000
	}
};

#define DEBUGLEVEL vehicleConfig.generalConfig.debugLevel

#endif