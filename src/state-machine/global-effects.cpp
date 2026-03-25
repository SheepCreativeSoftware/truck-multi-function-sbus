#include "global-effects.h"

GlobalEffects::GlobalEffects() 
	: blink(), leftTurnSignal(false), rightTurnSignal(false),  
	strobeSignal(false), strobeStartMillis(0),
	flashToPassSignal(false),
	corneringLeftSignal(false), corneringRightSignal(false),
	corneringLeftOffMillis(0), corneringRightOffMillis(0),
	beacon1position(0), beacon1previousMillis(0),
	beacon2position(0), beacon2previousMillis(0) {}

void GlobalEffects::updateTurnIndicators(uint32_t globalInputState) {
	if(globalInputState & BIT_HAZARD_LIGHT) {
		bool blinkerState = blink.blink(activeEffectsConfig.turnSignalFreq);
		leftTurnSignal = blinkerState;
		rightTurnSignal = blinkerState;
	} else if(globalInputState & BIT_TURN_SIGNAL_L) {
		leftTurnSignal = blink.blink(activeEffectsConfig.turnSignalFreq);
		rightTurnSignal = false;
	} else if(globalInputState & BIT_TURN_SIGNAL_R) {
		leftTurnSignal = false;
		rightTurnSignal = blink.blink(activeEffectsConfig.turnSignalFreq);
	} else if((leftTurnSignal || rightTurnSignal) && !blink.blink(activeEffectsConfig.turnSignalFreq)) {
		// Only turn lights off when they have made a full on cycle
		leftTurnSignal = false;
		rightTurnSignal = false;
	} else if (!leftTurnSignal && !rightTurnSignal) {
		blink.resetBlink();
	}
}

void GlobalEffects::updateStrobe(uint32_t globalInputState) {
	if(globalInputState & BIT_STROBE_LIGHT) {
		uint32_t currentMillis = millis();
		if(strobeStartMillis == 0) {
			strobeStartMillis = currentMillis;
		}

		uint32_t diff = currentMillis - strobeStartMillis;

		if(diff < activeEffectsConfig.strobeFlashDuration) {
			strobeSignal = true;
		} else if(diff < (
			activeEffectsConfig.strobeFlashDuration + 
			activeEffectsConfig.strobeShortPause
		)) {
			strobeSignal = false;
		} else if(diff < (
			activeEffectsConfig.strobeFlashDuration + 
			activeEffectsConfig.strobeShortPause + 
			activeEffectsConfig.strobeFlashDuration
		)) {
			strobeSignal = true;
		} else if(diff < (
			activeEffectsConfig.strobeFlashDuration + 
			activeEffectsConfig.strobeShortPause + 
			activeEffectsConfig.strobeFlashDuration + 
			activeEffectsConfig.strobeLongPause
		)) {
			strobeSignal = false;
		} else {
			strobeStartMillis = currentMillis;
		}

	} else {
		strobeSignal = false;
		strobeStartMillis = 0;
	}
};

void GlobalEffects::updateSingleBeacon(uint16_t beaconSpeed, uint8_t beaconMaxLeds, uint32_t* beaconPreviousMillis, uint8_t* beaconPosition) {
	uint32_t currentMillis = millis();
	uint32_t intervalPerLed = (uint32_t)beaconSpeed / (uint32_t)beaconMaxLeds;

	if(currentMillis - *beaconPreviousMillis >= intervalPerLed) {
		*beaconPreviousMillis = currentMillis;

		*beaconPosition += 1;
		if(*beaconPosition >= beaconMaxLeds) {
			*beaconPosition = 0;
		}
	}
};
void GlobalEffects::updateBeacon(uint32_t globalInputState) {
	if(globalInputState & BIT_BEACON_LIGHT) {
		updateSingleBeacon(activeEffectsConfig.beacon1Speed, activeEffectsConfig.beacon1MaxLeds, &beacon1previousMillis, &beacon1position);
	}

	if(globalInputState & BIT_BEACON_LIGHT) {
		updateSingleBeacon(activeEffectsConfig.beacon2Speed, activeEffectsConfig.beacon2MaxLeds, &beacon2previousMillis, &beacon2position);
	}
};

void GlobalEffects::updateFlashToPass(uint32_t globalInputState) {
	if(globalInputState & BIT_FLASH_TO_PASS) {
		if (millis() % activeEffectsConfig.flashToPassFreq < activeEffectsConfig.flashToPassFreq / 2) {
			flashToPassSignal = !flashToPassSignal;
		}
	} else {
		flashToPassSignal = false;
	}
}

void GlobalEffects::updateCornering(uint32_t globalInputState) {
	uint32_t now = millis();

	if(!(globalInputState & BIT_HAZARD_LIGHT)) {
		if(globalInputState & BIT_TURN_SIGNAL_L) {
			corneringLeftSignal = true;
			corneringRightSignal = false;
			corneringRightOffMillis = 0;
		}
	
		if(globalInputState & BIT_TURN_SIGNAL_R) {
			corneringRightSignal = true;
			corneringLeftSignal = false;
			corneringLeftOffMillis = 0;;
		}
	}

	if(globalInputState & BIT_STEERING_LEFT) {
		corneringLeftSignal = true;
		corneringRightSignal = false;
		corneringRightOffMillis = 0;
	}

	if(globalInputState & BIT_STEERING_RIGHT) {
		corneringRightSignal = true;
		corneringLeftSignal = false;
		corneringLeftOffMillis = 0;
	}

	if (corneringLeftSignal == true
		&& !(globalInputState & (BIT_STEERING_LEFT | BIT_TURN_SIGNAL_L))
	) {
		if(corneringLeftOffMillis == 0) {
			corneringLeftOffMillis = now;
		}
		if (corneringLeftOffMillis > 0 && now - corneringLeftOffMillis >= activeEffectsConfig.corneringLightOffDelay) {
			corneringLeftSignal = false;
			corneringLeftOffMillis = 0;
		}
	}

	if (corneringRightSignal == true
		&& !(globalInputState & (BIT_STEERING_RIGHT | BIT_TURN_SIGNAL_R))
	) {
		if(corneringRightOffMillis == 0) {
			corneringRightOffMillis = now;
		}
		if (corneringRightOffMillis > 0 && now - corneringRightOffMillis >= activeEffectsConfig.corneringLightOffDelay) {
			corneringRightSignal = false;
			corneringRightOffMillis = 0;
		}
	}
}

void GlobalEffects::update(uint32_t globalInputState) {
	updateTurnIndicators(globalInputState);
	updateStrobe(globalInputState);
	updateBeacon(globalInputState);
	updateFlashToPass(globalInputState);
	updateCornering(globalInputState);
};

uint16_t GlobalEffects::getActiveEffectMask() {
	uint16_t state = 0;

	if(leftTurnSignal) {
		state |= BIT_GLOBAL_TURN_L;
	}

	if(rightTurnSignal) {
		state |= BIT_GLOBAL_TURN_R;
	}

	if(strobeSignal) {
		state |= BIT_GLOBAL_STROBE;
	}

	if(flashToPassSignal) {
		state |= BIT_GLOBAL_FLASH_TO_PASS;
	}

	if(corneringLeftSignal) {
		state |= BIT_GLOBAL_CORNERING_L;
	}

	if(corneringRightSignal) {
		state |= BIT_GLOBAL_CORNERING_R;
	}

	return state;
};

uint8_t GlobalEffects::getBeaconPosition(uint8_t position) {
	switch (position) {
		case 1:
			return beacon1position;
		case 2: 
			return beacon2position;
		default:
			return 0;
	}
}
