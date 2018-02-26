
#pragma once


#include <inttypes.h>



/*
 * Facade to PPI device
 *
 * Connects signals from HW events to HW tasks
 */

class EventToTaskSignal {
public:
	static void connect(uint32_t * eventAddress, uint32_t * taskAddress);
};
