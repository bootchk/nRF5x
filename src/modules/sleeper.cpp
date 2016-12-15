
#include <cassert>

#include "sleeper.h"

// implementation needs
#include "nRFCounter.h"
#include "../drivers/mcu.h"


namespace {

// owns the mcu
MCU mcu;

// !!! not own TimerService
// Exclusive use of timer[0]
LongClockTimer* timerService;

OSTime maxSaneTimeout;

static ReasonForWake reasonForWake = None;





/*
 * IRQ handler callbacks
 *
 * Since there are two concurrent devices, there is a race to set reasonForWake
 *
 * !!! Note there may be other Timers which wake us but whose callbacks do not set reasonForWake.
 */
/*
 * Callbacks from IRQHandler, so keep short or schedule a task, queue work, etc.
 * Here we set flag that main event loop reads.
 *
 * Passing address, so names can be C++ mangled
 */

void rcvTimeoutTimerCallback() {
	if (reasonForWake == None)	// if msg didn't arrive just ahead of timeout, before main could cancel timeout
		reasonForWake = TimerExpired;
}

} // namespace







void Sleeper::init(
		OSTime maxAppTimeout,
		LongClockTimer* aLCT) {
	// assert a TimerService exists and is initialized (creating a Timer depends on it.)
	timerService = aLCT;	//receiveTimer.create(rcvTimeoutTimerCallback);
	maxSaneTimeout = maxAppTimeout;

	// assert(receiveTimerService.isOSClockRunning());
}


void Sleeper::sleepUntilEventWithTimeout(OSTime timeout) {
	// units are ticks, when RTC has zero prescaler: 30uSec

	// TODO should we be clearing, or asserting (rely on caller to clear, because of races?)
	clearReasonForWake();
	if (timeout < 5) {
		/*
		 * Less than minimum required by restartInTicks() of app_timer library.
		 * Don't sleep, but set reason for waking.
		 * I.E. simulate a sleep.
		 */
		reasonForWake = TimerExpired;
	}
	else { // timeout >= 5

		/*
		 * Sanity of SleepSync: never sleeps longer than two SyncPeriodDuration
		 */
		// TODO this should be a clamp, or throw
		assert(timeout <= maxSaneTimeout );

		// oneshot timer must not trigger before we sleep, else sleep forever
		timerService->startTimer(
				0,
				timeout,
				rcvTimeoutTimerCallback);
		sleepSystemOn();	// wake by received msg or timeout
		// assert IRQ
	}
	// We either never slept and simulated reasonForWake == Timeout,
	// or slept then woke and the handler (at SWI0 priority of APP_LOW) set reasonForWake in [Timeout, MsgReceived)
	// or an unexpected event woke us.
	// Because of the latter possiblity we can't assert(reasonForWake != Cleared);
}


void Sleeper::cancelTimeout(){
	timerService->cancelTimer(0);
	//receiveTimer.cancel();
}



void Sleeper::msgReceivedCallback() {
	/*
	 * If msg arrives after main read reasonForWake and before it stopped the receiver,
	 * this reason will go ignored and msg lost.
	 *
	 * If msg arrives immediately after a timeout but before main has read reasonForWake,
	 * the msg will be handled instead of the timeout.
	 */
	reasonForWake = MsgReceived;
}


ReasonForWake Sleeper::getReasonForWake() { return reasonForWake; }
void Sleeper::clearReasonForWake() { reasonForWake = None; }



void Sleeper::sleepSystemOn() {
	mcu.sleep();
}
