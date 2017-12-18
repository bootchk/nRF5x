#pragma once

#include <inttypes.h>

class Counter {
public:

	/*
	 * Has no init()
	 * Initialization of the source clock to Counter is done by LongClock.
	 */
	static void start();
	static void stop();
	static bool isTicking();

	static void configureOverflowInterrupt();
	static void clearOverflowEventAndWaitUntilClear();
	static bool isOverflowEvent();

	static uint32_t ticks();	// OSTime
};
