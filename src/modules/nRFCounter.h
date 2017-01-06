
#pragma once

#include <inttypes.h>
#include "../drivers/nvic.h"


/*
 * Type LongClock yields.  Only 56 bits are valid.
 */
typedef uint64_t LongTime;

typedef uint32_t OSTime;

enum TimerIndex {
	First =0,
	Second,
	Third
};

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

	static void init(Nvic*);
	static void reset();

	static LongTime nowTime();

	static void startTimer(
			TimerIndex index,	// [0:2]
			OSTime timeout, // [0:xffffff]
			void (*onTimeoutCallback)());

	static bool isTimerStarted(TimerIndex index);

	static void cancelTimer(TimerIndex index);

	static bool isOSClockRunning();
};
