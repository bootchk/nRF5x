
#include <cassert>

#include "sleeper.h"

// implementation needs
#include "nRFCounter.h"
#include "../drivers/mcu.h"


namespace {


MCU mcu;	// sleep()

// !!! not own
// Exclusive use of timer[SleepTimerIndex]
LongClockTimer* timerService;
TimerIndex SleepTimerIndex = First;	// Must not be used elsewhere

OSTime maxSaneTimeout;

ReasonForWake reasonForWake = None;





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
	if (timeout < LongClockTimer::MinTimeout) {
		/*
		 * Less than minimum required by restartInTicks() of app_timer library.
		 * Don't sleep, but set reason for waking.
		 * I.E. simulate a sleep.
		 */
		reasonForWake = TimerExpired;
	}
	else { // timeout >= the min that clock supports

		/*
		 * Sanity of SleepSync: never sleeps longer than two SyncPeriodDuration
		 */
		// TODO this should be a clamp, or throw
		assert(timeout <= maxSaneTimeout );

		// oneshot timer must not trigger before we sleep, else sleep forever
		timerService->startTimer(
				SleepTimerIndex,
				timeout,
				rcvTimeoutTimerCallback);
		mcu.sleep();
		// awakened by event: received msg or timeout or other

		/*
		 * If timer expired, timer is already stopped.
		 * Else, stop it to ensure consistent state.
		 * Note that for our timer semantics, it is safe to stop a timer that it not started,
		 * but not safe to start a timer that is already started.
		 */
		timerService->cancelTimer(SleepTimerIndex);
	}
	/*
	 * Cases:
	 * - never slept and simulated reasonForWake == Timeout
	 * - OR slept then woke and:
	 * -- IRQ handler set reasonForWake in [Timeout, MsgReceived)
	 * -- unexpected event woke us and reasonForWake is still None
	 *
	 * In all cases, assert timer is stopped (so using our timer semantics, it can be started again.)
	 *
	 * !!! Cannot assert that timeout amount of time has elapsed: unexpected events may wake early.
	 */
}


void Sleeper::cancelTimeout(){
	timerService->cancelTimer(SleepTimerIndex);
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


ReasonForWake Sleeper::getReasonForWake() {
	ReasonForWake result = reasonForWake;
	assert( result == None || result == TimerExpired || result == MsgReceived);
	return result;
}

void Sleeper::clearReasonForWake() { reasonForWake = None; }

