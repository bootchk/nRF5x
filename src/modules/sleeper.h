
#pragma once

#include "nRFCounter.h"


/*
 *
 * Responsibilities:
 * - sleep the system (wake on timeout or other event.)
 * - know reason for waking.
 * - know OSTime from OSClock
 * - know and enforce max timeout a sane app would ask for (currently an assertion, should be exception.)
 *
 * Uses a Clock/Timer.
 *
 * Specific other event that wakes: radio receives msg.
 *
 * Sleeping puts mcu to idle, low-power mode.
 * Note much power management is automatic by nrf52.
 * E.G. when sleep, all unused peripherals are powered off automatically.
 */



typedef enum {
	None = 2,
	MsgReceived,
	TimerExpired
} ReasonForWake;





class Sleeper {
public:
	// Public because passed to radio so it can hook IRQ into it
	static void msgReceivedCallback();

	/* maxSaneTimeout: max timeout a sane app should ask for. */
	static void init(
			OSTime maxSaneTimeout,
			LongClockTimer *);

	/* Sleep until any system event, and set a Timer that generates a waking event after timeout ticks. */
	static void sleepUntilEventWithTimeout(OSTime);

	/* Cancel Timer that would generate waking event. */
	static void cancelTimeout();

	// Not in-lined, used by external libraries
	static ReasonForWake getReasonForWake();
	static void clearReasonForWake();
};


