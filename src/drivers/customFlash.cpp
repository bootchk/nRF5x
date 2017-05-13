
#include <string.h>
#include <drivers/flashController.h>
#include "customFlash.h"


void CustomFlash::writeZeroAtIndex(FlagIndex index){
	writeIntAtIndex(index, 0);
}

void CustomFlash::writeIntAtIndex(FlagIndex index, int value){

	// assert index >=0 and <= 31

	FlashController::enableWrite();

	// Address arithmetic.  UICR is 32-bit words.
	*(uint32_t *)(FlashController::UICRStartAddress + index*4) = value ;

	FlashController::disableWrite();
}


void CustomFlash::copyStringToFlash(const char* functionName){

	FlashController::enableWrite();

	// Address arithmetic.  UICR is 32-bit words.
	// Assert function name is less than size of flash, say 1000 chars

	/*
	 * Cannot use strcpy() because UICR is not byte-addressable
	 *
	 * Hack: write only the first four chars.
	 * This assumes that functionName starts on a word boundary.
	 */
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString) = *(uint32_t*)(functionName);
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 4) = *(uint32_t*)(functionName+4);
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 8) = *(uint32_t*)(functionName+8);

	FlashController::disableWrite();
}
