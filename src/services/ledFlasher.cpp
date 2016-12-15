

#include "ledFlasher.h"

#include "../modules/nRFCounter.h"
#include "../modules/ledLogger.h"


namespace {

LongClockTimer* timerService;
LEDLogger ledLogger;

/*
 * Callback from timer, keep it short.
 * The timer interrupt wakes the mcu if was sleeping.
 */
void ledOffCallback() {
	ledLogger.switchLED(1, false);
	// Timer cancels itself
}



} // namespace

void LEDFlasher::init(LongClockTimer* aTimerService) {
	// require TimerService() started
	ledLogger.init();

	timerService = aTimerService;

	// ensure LEDs are configured
	// ensure LEDs are off
	// ensure timerService ready
}

void LEDFlasher::flashLED(unsigned int ordinal) {
	// amount==1, minimum amount
	flashLEDByAmount(ordinal, 1);
}

void LEDFlasher::flashLEDByAmount(unsigned int ordinal, unsigned int amount){
	// assert ledLogger initialized
	// assert TimerService initialized

	ledLogger.switchLED(ordinal, true);

	// start timer to turn LED off
	timerService->startTimer(
			Second,
			amount * MinTicksPerFlash,
			ledOffCallback
			);
}
