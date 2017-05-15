
#include "ledFlasher.h"

#include "../modules/ledService.h"
#include "../modules/longClockTimer.h"


namespace {

LongClockTimer* timerService;
LEDService* ledService;

/*
 * Callback from timer, keep it short.
 * The timer interrupt wakes the mcu if was sleeping.
 */
void ledOffCallback() {
	ledService->switchLED(1, false);
	// Timer cancels itself
}

} // namespace




void LEDFlasher::init(LongClockTimer* aTimerService, LEDService* aLedService) {
	// require TimerService started
	// require LedLogger initialized

	timerService = aTimerService;
	ledService = aLedService;
}




void LEDFlasher::flashLED(unsigned int ordinal) {
	// amount==1, minimum amount
	flashLEDByAmount(ordinal, 1);
}

void LEDFlasher::flashLEDByAmount(unsigned int ordinal, unsigned int amount){
	// assert LEDService initialized
	// assert TimerService initialized

	if (timerService->isTimerStarted(Second)) {
		// Led already flashing.
		// Illegal to startTimer already started.
		return;
	}

	ledService->switchLED(ordinal, true);



	// Clamp to max
	if (amount > MaxFlashAmount )
		amount = MaxFlashAmount;

	// Calculate timeout in units ticks from amount in units of min visible flash
	OSTime timeout = amount * MinVisibleTicksPerFlash;

	// start timer to turn LED off
	timerService->startTimer(
			Second,
			timeout,
			ledOffCallback
			);
}
