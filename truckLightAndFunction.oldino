/************************************ 
 * truckLightAndFunction v2.0.0
 * Date: 20.08.2025
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

/************************************
 * Configuration Programm
 ************************************/
#include "config.h"								// Configuration File
//#include "ioMapping.h"							// IO Mapping File
/************************************
 * Include Module and Library Files
 ************************************/

#include "readPPMdata.h"				// read Data from Buffer
#include "lightFunctions.h"
#include "starterBrightnessAdjustment.h"
#include "debugging.h"					// Handles debbuging info
#include "serialCommMaster.h"
#include <SerialIO.h>

struct MultiswitchChannel {
	uint16_t channel[8];
};

MultiswitchChannel multiswitch1;
MultiswitchChannel multiswitch2;

struct Lights {
	bool state;
	bool out;
	uint8_t brightness;
};

Lights parkLight;
Lights lowBeamLight;
Lights highBeamLight;
Lights highBeamLightFlash;
Lights leftIndicatorLight;
Lights rightIndicatorLight;
Lights fogLight;
Lights hazardLight;
Lights beaconLight;
Lights auxLight;
Lights brakeLight;
Lights reverseLight;

SerialCommMaster truckSerial;
StarterAdjustedBrightness brightnessAdjust;

uint8_t channel3Switch = 0;

//Setup Serial and check if Board is UNO with one Serial or Leonardo/Micro with to Serials
#ifdef HAVE_HWSERIAL1								//if serial ports 1 exist then the arduino has more than one serial port
	#ifndef SerialUSB								//if not allready defined
		#define SerialUSB SERIAL_PORT_MONITOR		//then define monitor port
	#endif
#else
	#define DEBUGLEVEL 1
#endif
#ifndef SerialHW									//if not allready defined
	#define SerialHW SERIAL_PORT_HARDWARE			//then define hardware port
#endif


void setup() {
	// put your setup code here, to run once:
	/************************************
	* Setup Inputs 
	************************************/
	pinMode(vehicleConfig.ppmConfig.pinChannel1, INPUT_PULLUP);
	pinMode(vehicleConfig.ppmConfig.pinChannel2, INPUT_PULLUP);
	pinMode(vehicleConfig.ppmConfig.pinSoundChannel, INPUT);
	pinMode(vehicleConfig.lightInputChannel.brakeSignal, INPUT_PULLUP);
	pinMode(vehicleConfig.lightInputChannel.reverseSignal, INPUT_PULLUP);
	/************************************
	* Setup Outputs 
	************************************/
	initLightOutput();
	setupLightOutput(
		vehicleConfig.parkingLight.outputPin,
		vehicleConfig.parkingLight.fadeOnTime,
		vehicleConfig.parkingLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.lowBeamLight.outputPin,
		vehicleConfig.lowBeamLight.fadeOnTime,
		vehicleConfig.lowBeamLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.highBeamLight.outputPin,
		vehicleConfig.highBeamLight.fadeOnTime,
		vehicleConfig.highBeamLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.fogLight.outputPin,
		vehicleConfig.fogLight.fadeOnTime,
		vehicleConfig.fogLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.frontLeftTurnLight.outputPin,
		vehicleConfig.frontLeftTurnLight.fadeOnTime,
		vehicleConfig.frontLeftTurnLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.frontRightTurnLight.outputPin,
		vehicleConfig.frontRightTurnLight.fadeOnTime,
		vehicleConfig.frontRightTurnLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.rearLeftTurnLight.outputPin,
		vehicleConfig.rearLeftTurnLight.fadeOnTime,
		vehicleConfig.rearLeftTurnLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.rearRightTurnLight.outputPin,
		vehicleConfig.rearRightTurnLight.fadeOnTime,
		vehicleConfig.rearRightTurnLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.reverseLight.outputPin,
		vehicleConfig.reverseLight.fadeOnTime,
		vehicleConfig.reverseLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.brakeLight.outputPin,
		vehicleConfig.brakeLight.fadeOnTime,
		vehicleConfig.brakeLight.fadeOffTime
	);
	setupLightOutput(
		vehicleConfig.auxLight.outputPin,
		vehicleConfig.auxLight.fadeOnTime,
		vehicleConfig.auxLight.fadeOffTime
	);

	// Initialize brightness adjustment
	brightnessAdjust.setupAdjustmentParameters(vehicleConfig.generalLightConfig.starterDimmingFactor, vehicleConfig.generalLightConfig.starterDimmingMultiplier);
	brightnessAdjust.configureBrightnessLevels(LightType::LOW_BEAM, LightModes::PRIMARY, vehicleConfig.lowBeamLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::LOW_BEAM, LightModes::SECONDARY, vehicleConfig.lowBeamLight.secondaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::LOW_BEAM, LightModes::TERTIARY, vehicleConfig.lowBeamLight.tertiaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::HIGH_BEAM, LightModes::PRIMARY, vehicleConfig.highBeamLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::PARKING, LightModes::PRIMARY, vehicleConfig.parkingLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::BRAKE, LightModes::PRIMARY, vehicleConfig.brakeLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::BRAKE, LightModes::SECONDARY, vehicleConfig.brakeLight.secondaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::REAR_LEFT_TURN, LightModes::PRIMARY, vehicleConfig.rearLeftTurnLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::REAR_LEFT_TURN, LightModes::SECONDARY, vehicleConfig.rearLeftTurnLight.secondaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::REAR_RIGHT_TURN, LightModes::PRIMARY, vehicleConfig.rearRightTurnLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::REAR_RIGHT_TURN, LightModes::SECONDARY, vehicleConfig.rearRightTurnLight.secondaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::FRONT_LEFT_TURN, LightModes::PRIMARY, vehicleConfig.frontLeftTurnLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::FRONT_RIGHT_TURN, LightModes::PRIMARY, vehicleConfig.frontRightTurnLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::REVERSE, LightModes::PRIMARY, vehicleConfig.reverseLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::FOG, LightModes::PRIMARY, vehicleConfig.fogLight.primaryOnBrightness);
	brightnessAdjust.configureBrightnessLevels(LightType::AUX, LightModes::PRIMARY, vehicleConfig.auxLight.primaryOnBrightness);

	/************************************
	* Setup Functions
	************************************/
	initInterrupts(vehicleConfig.ppmConfig.pinChannel1, vehicleConfig.ppmConfig.pinChannel2, vehicleConfig.ppmConfig.pinSoundChannel);
	if (DEBUGLEVEL >=1) {
		debuggingInit(DEBUGLEVEL, vehicleConfig.generalConfig.statusLightPin);
	}

	if (vehicleConfig.serialConfig.isEnabled) {
		truckSerial.begin(&SerialHW,			// Serial interface on arduino
					vehicleConfig.serialConfig.baudRate,
					vehicleConfig.serialConfig.byteFormat,
					vehicleConfig.serialConfig.timeout,
					vehicleConfig.serialConfig.pollingInterval,
					vehicleConfig.serialConfig.outTxEnablePin,
					vehicleConfig.serialConfig.protocolVersion
		);
	}
}

void loop() {                             // put your main code here, to run repeatedly:
	bool errorFlag = false;

	/*
	 * Read Switches and Potis from Multiswitches
	 * Some switches are commented as they are not yet in use.
	 */
	multiswitch1.channel[0] = getChannel1Switch(0, DIRECTION_DOWN);
	multiswitch1.channel[1] = getChannel1Switch(1, DIRECTION_MID);
	multiswitch1.channel[2] = getChannel1Switch(2, DIRECTION_DOWN);
	multiswitch1.channel[3] = getChannel1Switch(3, DIRECTION_UP);
	multiswitch1.channel[4] = getChannel1Switch(4, DIRECTION_DOWN);
	multiswitch1.channel[5] = getChannel1Switch(5, DIRECTION_MID);
	multiswitch1.channel[6] = getChannel1Switch(6, DIRECTION_DOWN);
	multiswitch1.channel[7] = getChannel1Switch(7, DIRECTION_DOWN);

	multiswitch2.channel[0] = getChannel2Poti(0, 1520);
	multiswitch2.channel[1] = getChannel2Poti(1, 1520);
	multiswitch2.channel[2] = getChannel2Switch(2, DIRECTION_MID);
	multiswitch2.channel[3] = getChannel2Switch(3, DIRECTION_MID);
	multiswitch2.channel[4] = getChannel2Switch(4, DIRECTION_MID);
	multiswitch2.channel[5] = getChannel2Switch(5, DIRECTION_MID);
	multiswitch2.channel[6] = getChannel2Switch(6, DIRECTION_MID);
	multiswitch2.channel[7] = getChannel2Switch(7, DIRECTION_MID);

	channel3Switch = getChannel3Signal();

	/*
	 * Map switches to Functions
	 */
	
	parkLight.state = mapSwitchToFunction(multiswitch1.channel[0], false, true, true);	// Function to map a Key [Down, Mid, Up]
	lowBeamLight.state = mapSwitchToFunction(multiswitch1.channel[0], false, false, true);	// Function to map a Key [Down, Mid, Up]
	leftIndicatorLight.state = mapSwitchToFunction(multiswitch1.channel[1], true, false, false);	// Function to map a Key [Down, Mid, Up]
	rightIndicatorLight.state = mapSwitchToFunction(multiswitch1.channel[1], false, false, true);	// Function to map a Key [Down, Mid, Up]
	fogLight.state = mapSwitchToFunction(multiswitch1.channel[2], false, false, true);	// Function to map a Key [Down, Mid, Up]
	hazardLight.state = mapSwitchToFunction(multiswitch1.channel[3], false, false, true);	// Function to map a Key [Down, Mid, Up]
	beaconLight.state = mapSwitchToFunction(multiswitch1.channel[4], false, false, true);	// Function to map a Key [Down, Mid, Up]
	highBeamLight.state = mapSwitchToFunction(multiswitch1.channel[5], true, false, false);	// Function to map a Key [Down, Mid, Up]
	highBeamLightFlash.state = mapSwitchToFunction(multiswitch1.channel[5], false, false, true);	// Function to map a Key [Down, Mid, Up]
	auxLight.state = mapSwitchToFunction(multiswitch1.channel[6], false, false, true);	// Function to map a Key [Down, Mid, Up]
	reverseLight.state = !digitalRead(vehicleConfig.lightInputChannel.reverseSignal);	// Function to read the Reverse Signal from the External Controller
	brakeLight.state = !digitalRead(vehicleConfig.lightInputChannel.brakeSignal);

	/*
	 * Write Light function state to the output var
	 */
	parkLight.out = directlyToOutput(parkLight.state);
	lowBeamLight.out = directlyToOutput(lowBeamLight.state);
	highBeamLight.out = highBeamFlash(highBeamLight.state, highBeamLightFlash.state, vehicleConfig.highBeamConfig.flashFrequency);
	fogLight.out = directlyToOutput(fogLight.state);
	beaconLight.out = directlyToOutput(beaconLight.state);
	auxLight.out = directlyToOutput(auxLight.state);
	reverseLight.out = directlyToOutput(reverseLight.state);
	brakeLight.out = directlyToOutput(brakeLight.state);

	setTurnIndicators(
		leftIndicatorLight.state,
		rightIndicatorLight.state,
		hazardLight.state,
		&leftIndicatorLight.out,
		&rightIndicatorLight.out,
		vehicleConfig.turnSignalConfig.flashFrequency
	);

	bool isStarterActive = false;
	if(channel3Switch == DIRECTION_DOWN) isStarterActive = true;
	brightnessAdjust.setStarterState(isStarterActive);

	/*
	 * Set Outputs
	 */

	setBooleanLight(
		vehicleConfig.parkingLight.outputPin,
		parkLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::PARKING)
	);
	
	if (vehicleConfig.lowBeamConfig.isParkingLight && vehicleConfig.lowBeamConfig.isHighBeam) {
		setCombinedHeadlightAll(
			vehicleConfig.lowBeamLight.outputPin,
			parkLight.out,
			lowBeamLight.out,
			highBeamLight.out,
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM, LightModes::TERTIARY),
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM, LightModes::SECONDARY),
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM)
		);
	} else if (vehicleConfig.lowBeamConfig.isHighBeam) {
		setCombinedHeadlightHighOnly(
			vehicleConfig.lowBeamLight.outputPin,
			lowBeamLight.out,
			highBeamLight.out,
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM, LightModes::SECONDARY),
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM)
		);
	} else if (vehicleConfig.lowBeamConfig.isParkingLight) {
		setCombinedHeadlightParkOnly(
			vehicleConfig.lowBeamLight.outputPin,
			parkLight.out,
			lowBeamLight.out,
			
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM, LightModes::SECONDARY),
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM)
		);
	} else {
		setBooleanLight(
			vehicleConfig.lowBeamLight.outputPin,
			lowBeamLight.out, 
			brightnessAdjust.getBrightnessLevel(LightType::LOW_BEAM)
		);
	}
	
	setBooleanLight(
		vehicleConfig.highBeamLight.outputPin, 
		highBeamLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::HIGH_BEAM)
	);
	setBooleanLight(
		vehicleConfig.fogLight.outputPin, 
		fogLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::FOG)
	);
	setBooleanLight(
		vehicleConfig.auxLight.outputPin, 
		auxLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::AUX)
	);
	setBooleanLight(
		vehicleConfig.frontLeftTurnLight.outputPin, 
		leftIndicatorLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::FRONT_LEFT_TURN)
	);
	setBooleanLight(
		vehicleConfig.frontRightTurnLight.outputPin, 
		rightIndicatorLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::FRONT_RIGHT_TURN)
	);
	setBooleanLight(
		vehicleConfig.reverseLight.outputPin, 
		reverseLight.out, 
		brightnessAdjust.getBrightnessLevel(LightType::REVERSE)
	);

	switch (vehicleConfig.generalConfig.countryOption) {
	case CountryOption::EU:
		setBooleanLight(
			vehicleConfig.rearLeftTurnLight.outputPin,
			leftIndicatorLight.out, 
			brightnessAdjust.getBrightnessLevel(LightType::REAR_LEFT_TURN)
		);
		setBooleanLight(
			vehicleConfig.rearRightTurnLight.outputPin,
			rightIndicatorLight.out,
			brightnessAdjust.getBrightnessLevel(LightType::REAR_RIGHT_TURN)
		);
		setBooleanLight(
			vehicleConfig.brakeLight.outputPin,
			brakeLight.out,
			brightnessAdjust.getBrightnessLevel(LightType::BRAKE)
		);
		break;

	case CountryOption::US:
		if (leftIndicatorLight.state || hazardLight.state) {
			setBooleanLight(
				vehicleConfig.rearLeftTurnLight.outputPin,
				leftIndicatorLight.out, 
				brightnessAdjust.getBrightnessLevel(LightType::REAR_LEFT_TURN)
			);
		} else if (leftIndicatorLight.out == false) {
			setBrakingWithPark(
				vehicleConfig.rearLeftTurnLight.outputPin,
				parkLight.state, 
				brakeLight.state,
				brightnessAdjust.getBrightnessLevel(LightType::REAR_LEFT_TURN, LightModes::SECONDARY),
				brightnessAdjust.getBrightnessLevel(LightType::REAR_LEFT_TURN, LightModes::PRIMARY)
			);
		}

		if (rightIndicatorLight.state || hazardLight.state) {
			setBooleanLight(
				vehicleConfig.rearRightTurnLight.outputPin,
				rightIndicatorLight.out, 
				brightnessAdjust.getBrightnessLevel(LightType::REAR_RIGHT_TURN)
			);
		} else if (rightIndicatorLight.out == false) {
			setBrakingWithPark(
				vehicleConfig.rearRightTurnLight.outputPin,
				parkLight.state,
				brakeLight.state,
				brightnessAdjust.getBrightnessLevel(LightType::REAR_RIGHT_TURN, LightModes::SECONDARY),
				brightnessAdjust.getBrightnessLevel(LightType::REAR_RIGHT_TURN, LightModes::PRIMARY)
			);
		}

		setBrakingWithPark(
			vehicleConfig.brakeLight.outputPin,
			parkLight.state,
			brakeLight.state,
			brightnessAdjust.getBrightnessLevel(LightType::BRAKE, LightModes::SECONDARY),
			brightnessAdjust.getBrightnessLevel(LightType::BRAKE, LightModes::PRIMARY)
		);
		break;
	}

	if (vehicleConfig.serialConfig.isEnabled == true) {
		/*
		* Setup serial communication
		*/
		truckSerial.setLightData(LightIdentifier::PARK_LIGHT, parkLight.out);
		truckSerial.setLightData(LightIdentifier::BRAKE_LIGHT, brakeLight.out);
		truckSerial.setLightData(LightIdentifier::REVERSE_LIGHT, reverseLight.out);
		truckSerial.setLightData(LightIdentifier::RIGHT_BLINK, rightIndicatorLight.out);
		truckSerial.setLightData(LightIdentifier::LEFT_BLINK, leftIndicatorLight.out);
		truckSerial.setLightData(LightIdentifier::AUX_LIGHT, auxLight.out);
		truckSerial.setLightData(LightIdentifier::BEACON_LIGHT, beaconLight.out);
		truckSerial.setLightData(LightIdentifier::DIMM_LIGHTS, isStarterActive);
		truckSerial.setAdditionalData(AdditionalDataIdentifier::LEFT_TURN_INDICATOR, leftIndicatorLight.state);
		truckSerial.setAdditionalData(AdditionalDataIdentifier::RIGHT_TURN_INDICATOR, rightIndicatorLight.state);
		truckSerial.setAdditionalData(AdditionalDataIdentifier::HAZARD_STATE, hazardLight.state);
		//truckSerial.setAdditionalData(AdditionalDataIdentifier::SERVO_POSITION_DOWN, multiswitch2.channel[1] == DIRECTION_DOWN);
		//truckSerial.setAdditionalData(AdditionalDataIdentifier::SERVO_POSITION_UP, multiswitch2.channel[1] == DIRECTION_UP);
		truckSerial.setServoData(ServoDataIdentifier::SERVO_CHANNEL_1, multiswitch2.channel[0]);
		truckSerial.setServoData(ServoDataIdentifier::SERVO_CHANNEL_2, multiswitch2.channel[1]);
		truckSerial.update();
	}

	/*
	 * Setup Debugging
	 */

	if (DEBUGLEVEL >=1) {
		controllerStatus(errorFlag, vehicleConfig.generalConfig.statusLightPin);
	}

	switch (DEBUGLEVEL) {
		case DebugLevel::NONE:
			// No debugging information
			break;
		case DebugLevel::STATUS_ONLY:
			// Handled below
			break;
		case DebugLevel::PPM_CHANNEL1:
			debugChannelEvaluation(
				1,
				multiswitch1.channel[0],
				multiswitch1.channel[1],
				multiswitch1.channel[2],
				multiswitch1.channel[3],
				multiswitch1.channel[4],
				multiswitch1.channel[5],
				multiswitch1.channel[6],
				multiswitch1.channel[7]
			);
			break;
		case DebugLevel::PPM_CHANNEL2:
			debugChannelEvaluation(
				2,
				multiswitch2.channel[0],
				multiswitch2.channel[1],
				multiswitch2.channel[2],
				multiswitch2.channel[3],
				multiswitch2.channel[4],
				multiswitch2.channel[5],
				multiswitch2.channel[6],
				multiswitch2.channel[7]
			);
			break;
		case DebugLevel::FUNCTION_STATE:
			debugFunctionState(parkLight.state,
								lowBeamLight.state,
								highBeamLight.state,
								highBeamLightFlash.state,
								fogLight.state,
								beaconLight.state,
								auxLight.state,
								hazardLight.state,
								leftIndicatorLight.state,
								rightIndicatorLight.state,
								reverseLight.state,
								brakeLight.state);
			break;
		case DebugLevel::FUNCTION_OUT:
			debugFunctionOut(parkLight.out,
								lowBeamLight.out,
								highBeamLight.out,
								highBeamLightFlash.out,
								fogLight.out,
								beaconLight.out,
								auxLight.out,
								hazardLight.out,
								leftIndicatorLight.out,
								rightIndicatorLight.out,
								reverseLight.state,
								brakeLight.state);
			break;
		case DebugLevel::STARTER_DIMMING:
			SerialUSB.println(isStarterActive);
			SerialUSB.println(digitalRead(vehicleConfig.ppmConfig.pinSoundChannel));
			break;
		}
}