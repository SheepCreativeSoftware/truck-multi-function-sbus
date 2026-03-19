#include "global-effects.h"

GlobalEffects::GlobalEffects() 
	: leftTurnSignal(false), rightTurnSignal(false), blink(), 
	strobeSignal(false), strobeStartMillis(0),
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

void GlobalEffects::update(uint32_t globalInputState) {
	updateTurnIndicators(globalInputState);
	updateStrobe(globalInputState);
	updateBeacon(globalInputState);
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
