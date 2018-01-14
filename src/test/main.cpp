
/*
 * Test main for nRF5x library.
 *
 * Minimal: only tests the LongClock.
 * This project is configured to build a library, and not an executable: this main is an example, non-functional.
 *
 * For more tests, see testCounter project.
 * That project is configured to build for a target nRF5x chip.
 *
 * Copyright 2016 lloyd konneker
 */


#include <clock/longClock.h>
#include "radio/radio.h"

#include "drivers/nvic.h"
#include "drivers/powerSupply.h"
#include "drivers/oscillators/hfClock.h"

#include "services/logger.h"



// TODO no instance needed
Nvic nvic;
RTTLogger logger;


HfCrystalClock hfClock;
DCDCPowerSupply powerSupply;
Radio radio;

void msgReceivedCallback();
void msgReceivedCallback() {
	logger.log("Msg received\n");
}



int main() {
	// assert embedded system startup is done and calls main.

	// TODO this is obsolete, FIXME
	LongClock::start();

	radio.configure();

	radio.setMsgReceivedCallback(msgReceivedCallback);
	logger.log("Initialized clock and radio\n");

	while (true) {
		// delay
		logger.log( LongClock::nowTime() );
		// longClock.timer1.start(100);
	}

	return 0;
}
