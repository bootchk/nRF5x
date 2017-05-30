
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


void CustomFlash::copyStringPrefixToFlash(const char* text){

	FlashController::enableWrite();

	// Address arithmetic.  UICR is 32-bit words.
	// Write only prefix of text
	// Size of flash, say 16 chars

	/*
	 * Cannot use strcpy() because UICR is not byte-addressable
	 *
	 * Hack: write only the first 16 chars.
	 * This assumes that text starts on a word boundary?
	 * This assumes text is longer than 16 chars
	 */
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString) = *(uint32_t*)(text);
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 4) = *(uint32_t*)(text+4);
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 8) = *(uint32_t*)(text+8);
	*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 12) = *(uint32_t*)(text+8);

	FlashController::disableWrite();
}
