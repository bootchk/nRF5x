
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



#include "modules/nRFCounter.h"
#include "drivers/nvic.h"
#include "services/logger.h"



LongClockTimer longClockTimer;
Nvic nvic;

int main() {
	// assert embedded system startup is done and calls main.

	longClockTimer.init(&nvic);

	while (true) {
		// delay
		logLongLong( longClockTimer.getNowTime() );
		// longClockTimer.timer1.start(100);
	}

	return 0;
}
