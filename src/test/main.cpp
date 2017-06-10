
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


#include <modules/longClockTimer.h>
#include "radio/radio.h"

#include "drivers/nvic.h"
#include "drivers/powerSupply.h"
#include "drivers/hfClock.h"

#include "services/logger.h"



LongClockTimer longClockTimer;
Nvic nvic;


HfCrystalClock hfClock;
DCDCPowerSupply powerSupply;
Radio radio;

void msgReceivedCallback();
void msgReceivedCallback() {
	log("Msg received\n");
}



int main() {
	// assert embedded system startup is done and calls main.



	longClockTimer.init(&nvic);

	radio.init(
			&nvic,
			&powerSupply,
			&hfClock
	);

	radio.setMsgReceivedCallback(msgReceivedCallback);
	log("Initialized clock and radio\n");

	while (true) {
		// delay
		logLongLong( longClockTimer.nowTime() );
		// longClockTimer.timer1.start(100);
	}

	return 0;
}
