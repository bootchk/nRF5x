
#include <modules/longClockTimer.h>
#include <cassert>

#include "sleeper.h"

// implementation needs
#include "../drivers/mcu.h"


namespace {

const TimerIndex SleepTimerIndex = First;	// Must not be used elsewhere


// !!! not own
// Exclusive use of timer[SleepTimerIndex]
LongClockTimer* timerService;

OSTime maxSaneTimeout = LongClockTimer::MaxTimeout;	// defaults to max a Timer allows

ReasonForWake reasonForWake = ReasonForWake::Cleared;





/*
 * Callback from RTC_IRQ which dispatches on event: sleep timer compare register expire, other timer compare register expire, and overflow.
 * This callback is called for all those events.
 * !!! May be other Timers which wake mcu and whose callbacks are called as well as this, but those callbacks do not set reasonForWake.
 *
 * Here we set flag that main event loop reads so that the user of this Timer (who is sleeping) knows reason for wake.
 * Since there are two concurrent devices (radio and counter), there is a race to set reasonForWake.
 * Here we prioritize.
 *
 * This callback is from within IRQHandler with nested interrupts precluded, so keep short or schedule a task, queue work, etc.
 *
 * We pass address around, so names can be C++ mangled
 */

void timerIRQCallback(TimerInterruptReason reason) {
	switch(reason) {
	case SleepTimerCompare:
		// Prioritize reasonForWake
		switch(reasonForWake) {
		case ReasonForWake::Cleared:
		case ReasonForWake::Unknown:
		case ReasonForWake::BrownoutWarning:
		case ReasonForWake::CounterOverflowOrOtherTimerExpired:
			// Higher priority reason
			reasonForWake = ReasonForWake::SleepTimerExpired;
			// TODO assert that Timer current Count - Timer starting count == timeout
			break;
		case ReasonForWake::MsgReceived:
			// Do not overwrite highest priority: MsgReceived
			break;
		case ReasonForWake::SleepTimerExpired:
		case ReasonForWake::HFClockStarted:
			assert(false);	// Timer was started again before handling/clearing previous expiration.
			// Or unexpected HFClockStart
		}
		break;

	case OverflowOrOtherTimerCompare:
		/*
		 * Awakened, but not for First Timer.
		 * Overflow and OtherTimer can come even if a reason is already set,
		 * since the interrupt is always enabled for Overflow,
		 * and interrupt for other timers is periodically enabled.
		 * XXX simpler to use separate peripheral for other timers.
		 */
		switch(reasonForWake) {
		case ReasonForWake::Cleared:
			reasonForWake = ReasonForWake::CounterOverflowOrOtherTimerExpired;
			break;

		case ReasonForWake::BrownoutWarning:
		case ReasonForWake::Unknown:
		case ReasonForWake::MsgReceived:
		case ReasonForWake::SleepTimerExpired:
		case ReasonForWake::CounterOverflowOrOtherTimerExpired:
			// Reason is already higher priority
			break;
		case ReasonForWake::HFClockStarted:
			assert(false);	// Design does not user timer while HF clock is starting
		}
	}
	// assert reasonForWake is not Cleared
}

} // namespace







void Sleeper::init(LongClockTimer* aLCT) {
	// assert a TimerService exists and is initialized (creating a Timer depends on it.)
	timerService = aLCT;	//receiveTimer.create(rcvTimeoutTimerCallback);
	// assert(receiveTimerService.isOSClockRunning());
}

void Sleeper::setSaneTimeout(OSTime maxAppTimeout) {
	maxSaneTimeout = maxAppTimeout;
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
		reasonForWake = ReasonForWake::SleepTimerExpired;
	}
	else { // timeout >= the min that clock supports

		/*
		 * Sanity of SleepSync: never sleeps longer than two SyncPeriodDuration
		 */
		// TODO this should be a clamp, or throw
		assert(timeout <= maxSaneTimeout );

		/*
		 * oneshot timer must not trigger before we sleep, else sleep forever.
		 * Not using WDT to guard against that.
		 */
		timerService->startTimer(
				SleepTimerIndex,
				timeout,
				timerIRQCallback);
		MCU::sleep();
		/*
		 * awakened by event: received msg or timeout or other event.
		 * !!! Other timer expirations may wake us.
		 * But other timer expirations won't call self's callback.
		 */

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
	 * -- RTC or RADIO IRQ handler set reasonForWake in [Timeout, MsgReceived)
	 * -- RTC IRQ for another Timer and reasonForWake is still none
	 * -- unexpected event woke us and reasonForWake is still None
	 *
	 * In all cases, assert timer is stopped (so using our timer semantics, it can be started again.)
	 *
	 * !!! Cannot assert that timeout amount of time has elapsed: other clock events may wake early.
	 */
}


void Sleeper::cancelTimeout(){
	timerService->cancelTimer(SleepTimerIndex);
}



void Sleeper::sleepUntilEvent(ReasonForWake reason){
	/*
	 * sleep until IRQ signals started event.
	 * !!! Other interrupts (brownout, clock overflow, led Timer 2 etc. may wake the sleep.)
	 * Other interrupts may increase time between start() and sleep().
	 * The max start time is 360uSec NRF52,
	 * so there should be plenty of time to get asleep before interrupt occurs.
	 */

	while (Sleeper::getReasonForWake() != reason) {
		/*
		 * !!! Event must come, else infinite loop.
		 * Ignore other events from clock and other sources
		 * e.g. timers for flashing LED's (whose interrupts will occur, but returns to this thread.)
		 */
		MCU::sleep();
		// TODO examine other reasons here and log abnormal ones such as brownout
	}
}



/*
 * Called by RadioIRQ.
 * Not assert packet is valid.
 */
void Sleeper::msgReceivedCallback() {
	/*
	 * If msg arrives after main read reasonForWake and before it stopped the receiver,
	 * this reason will go ignored and msg lost.
	 *
	 * If msg arrives immediately after a timeout but before main has read reasonForWake,
	 * the msg will be handled instead of the timeout.
	 */
	reasonForWake = ReasonForWake::MsgReceived;
}


/*
 * reasonForWake  is an enum class and compile time checked valid
 */
ReasonForWake Sleeper::getReasonForWake() { return reasonForWake; }
void Sleeper::setReasonForWake(ReasonForWake reason) { reasonForWake = reason; }




void Sleeper::clearReasonForWake() { reasonForWake = ReasonForWake::Cleared; }

