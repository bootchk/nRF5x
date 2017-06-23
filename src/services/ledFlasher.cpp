
#include <cassert>

#include "ledFlasher.h"

#include "../modules/ledService.h"
// implementation uses pure class LongClock already included by header
#include "../clock/timer.h"



namespace {

LEDService* ledService;

/*
 * Callback from timer, keep it short.
 * The timer interrupt wakes the mcu if was sleeping.
 */
void ledOffCallback(TimerInterruptReason reason) {
	// We don't callback for Overflow or other timers
	// TODO SleepTimerExpired?
	assert(reason == OverflowOrOtherTimerCompare);

	// TODO we should turn off led that is on, here assume it is first LED, ordinal 1
	ledService->switchLED(1, false);
	// Timer cancels itself
}

} // namespace




void LEDFlasher::init(LEDService* aLedService) {
	// TODO require Timer is init
	// assert(LongClockTimer::isOSClockRunning());
	// require LedLogger initialized

	ledService = aLedService;
}




void LEDFlasher::flashLEDMinimumVisible(unsigned int ordinal) {
	flashLEDByAmount(ordinal, MinAmountToFlash);
}

void LEDFlasher::flashLEDByAmount(unsigned int ordinal, unsigned int amount){
	// assert LEDService initialized
	// assert TimerService initialized
	assert(amount>=MinAmountToFlash);
	if (Timer::isStarted(Second)) {
		// Led already flashing.
		// Illegal to start Timer already started.
		return;
	}

	ledService->switchLED(ordinal, true);



	// Clamp to max
	if (amount > MaxFlashAmount )
		amount = MaxFlashAmount;

	// Calculate timeout in units ticks from amount in units of min visible flash
	OSTime timeout = amount * MinVisibleTicksPerFlash;

	// start timer to turn LED off
	Timer::start(
			Second,
			timeout,
			ledOffCallback
			);
}
