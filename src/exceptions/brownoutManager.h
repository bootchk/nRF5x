
#pragma once

#include <inttypes.h>

typedef uint32_t (*BrownoutCallback)();

/*
 * Knows how to write important info to flash when brownout happens.
 *
 * App is required to define one, since an IRQ definitely calls it.
 */


class BrownoutManager {
public:
	/*
	 * Write to flash if not already written:
	 *  - result of callback if callback registered
	 *  - else faultAddress
	 *
	 * Called in a brownout state (EVENTS_POFWARN is set)
	 */
	void recordToFlash(uint32_t faultAddress);

	/*
	 * Register function that returns important information at time of brownout.
	 */
	void registerCallback(BrownoutCallback);
};

