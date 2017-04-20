

/*
 * Override the function defined in newlib nano.
 * To catch soft exceptions.
 *
 * assert calls abort() and then _exit()
 *
 * The _exit() defined in newlib nano infinite loops.
 * For an embedded app with nanopower, better to indicate fault and then sleep.
 */

#include "../drivers/customFlash.h"
#include "../exceptions/faultHandlers.h"


extern "C"

__attribute__((noreturn))
void _exit() {

	// word 0 reserved to indicate exit was called from assertion
	CustomFlash::writeZeroAtIndex(0);

	sleepForeverInLowPower();
}

