
#include "brownoutHandler.h"

#include "../drivers/customFlash.h"
#include "../drivers/powerComparator.h"

void brownoutWritePCToFlash(uint32_t faultAddress) {
	/*
	 * Since for some power supplies, the system may repeatedly brownout and POR,
	 * and since flash is not writeable more than once,
	 * only write PC if not written already.
	 */

	/*
	 * !!! First must counteract HW lock of flash during brownout.
	 * Disable comparator so it doesn't trigger again when we clear the event.
	 */
	PowerComparator::disable();
	PowerComparator::clearPOFEvent();
	// assert NVMC is not HW locked against writes

	/*
	 * This takes power, and up to 300uSeconds.
	 * It might not succeed in writing to flash, since power is failing
	 */
	CustomFlash::tryWriteIntAtIndex(BrownoutPCIndex, faultAddress);

	/*
	 * This is not designed rigorously for the continuation.
	 * The PowerComparator is left in a state different from entry and will not catch further brownouts.
	 * The usual continuation is to infinite loop.
	 * If you want a clean continuation, need code here to restore PowerComparator.
	 * It is reasonable to want a clean continuation,
	 * because the detected brownout is only potential; might not become a real BOR reset.
	 * That is, power might recover.
	 */
}
