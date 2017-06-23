#pragma once

#include <inttypes.h>

// Uses Nvic class


/*
 * Type the Counter yields.  Only 24 bits are valid.
 */
typedef uint32_t OSTime;



class Counter {
public:

	/*
	 * Has no init()
	 * Initialization of the source clock to Counter is done by LongClock.
	 */
	static void start();
	static void stop();

	static void configureOverflowInterrupt();
	static void clearOverflowEvent();
	static bool isOverflowEvent();

	static OSTime ticks();
};
