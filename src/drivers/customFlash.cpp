
#include <drivers/flashController.h>
#include "customFlash.h"


void CustomFlash::writeZeroAtIndex(unsigned int index){

	// assert index >=0 and <= 31

	FlashController::enableWrite();

	// Address arithmetic.  UICR is 32-bit words.
	*(uint32_t *)(FlashController::UICRStartAddress + index*4) = 0 ;

	FlashController::disableWrite();
}
