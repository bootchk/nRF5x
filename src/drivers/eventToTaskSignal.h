
#pragma once


#include <inttypes.h>



/*
 * Facade to PPI device
 *
 * Connects signals from HW events to HW tasks
 */

class EventToTaskSignal {
public:
	/*
	 * Connection always enabled.
	 */
	static void connect(uint32_t * eventAddress, uint32_t * taskAddress);

	/*
	 * Connection one-shot: disables itself after triggering, and must be reenabled.
	 */
	static void connectOneShot(uint32_t * eventAddress, uint32_t * taskAddress);

	static void enableOneShot();
};
