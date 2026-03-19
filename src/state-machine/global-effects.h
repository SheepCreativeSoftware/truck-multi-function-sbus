#pragma once
#include <Arduino.h>
#include "blink.h"
#include "../config/light-mode.h"
#include "../config/global-effects-config.h"

class GlobalEffects {
	private:
	Blink blink;
	bool leftTurnSignal;
	bool rightTurnSignal;

	bool strobeSignal;
	uint32_t strobeStartMillis;

	uint8_t beacon1position;
	uint32_t beacon1previousMillis;
	uint8_t beacon2position;
	uint32_t beacon2previousMillis;
	
	void updateTurnIndicators(uint32_t globalInputState);
	void updateStrobe(uint32_t globalInputState);
	void updateSingleBeacon(uint16_t beaconSpeed, uint8_t beaconMaxLeds, uint32_t* beaconPreviousMillis, uint8_t* beaconPosition);
	void updateBeacon(uint32_t globalInputState);
	public:
	GlobalEffects();

	void update(uint32_t globalInputState);
	uint16_t getActiveEffectMask();
	uint8_t getBeaconPosition(uint8_t position);
};