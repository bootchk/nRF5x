
#pragma once

#include <inttypes.h>
#include "../drivers/nvic.h"


/*
 * Type LongClock yields.  Only 56 bits are valid.
 */
typedef uint64_t LongTime;

typedef uint32_t OSTime;



/*
 * Enum Timer instances.
 */
// Future: better class with + operator and use it for iterating
enum TimerIndex {
	First =0,
	Second,
	Third
};

/*
 * Reason for interrupt.
 * Users of Timer that sleep on interrupt (WFI) may want to know when wake but timer not expired.
 */
enum TimerInterruptReason {
	OverflowOrOtherTimerCompare,
	SleepTimerCompare
};

// Type of function called back when there is an interrupt from RTC
typedef void (*TimerCallback)(TimerInterruptReason);



/*
 * Thin wrapper around RTC device of Nordic nRF52/51.
 * This defines an API that might be portable.
 *
 * Owns private data defined in anon namespace.
 *
 * Provides:
 * - LongClock 56-bit counter. At resolution of 30uSec, overflows in 67,000 years.
 * - Three Timers, 24-bit, one-shot. At resolution of 30uSec, max timeout is 5 seconds.
 *
 * !!! Timers are shorter than the LongClock.
 * 24-bit RTC device counter underlies both LongClock and Timers,
 * but LongClock extends the clock to 56-bits.
 *
 * Legal call sequences:
 *
 * Normal:  init, getNowTime, startTimer, cancelTimer
 * You can cancel a Timer that has not been started: init, cancelTimer
 */
class LongClockTimer {
	static const int OSClockCountBits = 24;

public:
	// Not support timeouts longer than compare register
	static const unsigned int MaxTimeout = 0xFFFFFF;

	// Device won't reliably cause event for timeouts < 2
	// Some authors use 3 for safety
	// app_timer used 5, because it had other delays?
	static const unsigned int MinTimeout = 2;

	/*
	 * How many Timers this device supports (with compare registers).
	 * Depends on device, here 3 is compatible with nRF51 and nRF52, certain revisions?
	 */
	static const unsigned int CountTimerInstances = 3;

	/*
	 * init does not guarantee clock is running (only that it is started.)
	 * You can start a timer even if clock is not running, but it may delay longer than requested,
	 * until oscillator isRunning and thus isOSClockRunning().
	 *
	 * If you require accurate timer, start a dummy timer before you start an accurate timer.
	 */
	static void init(Nvic*);

	/*
	 * Make high order bits zero.
	 * Low order bits are whatever the HW counter is.
	 */
	static void resetToNearZero();

	static LongTime nowTime();

	static void startTimer(
			TimerIndex index,	// [0:2]
			OSTime timeout, // [0:0xffffff]
			TimerCallback onTimeoutCallback);

	static bool isTimerStarted(TimerIndex index);

	static void cancelTimer(TimerIndex index);

	static bool isOSClockRunning();
};
