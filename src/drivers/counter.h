#pragma once

#include <inttypes.h>

#include "../types.h"  // OSTime

// Uses Nvic class




class Counter {
public:

	/*
	 * Has no init()
	 * Initialization of the source clock to Counter is done by LongClock.
	 */
	static void start();
	static void stop();

	static void configureOverflowInterrupt();
	static void clearOverflowEventAndWaitUntilClear();
	static bool isOverflowEvent();

	static OSTime ticks();
};
