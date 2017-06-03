
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

bool CustomFlash::isWrittenAtIndex(FlagIndex index) {
	// Can read without enabling controller
	return *(uint32_t *)(FlashController::UICRStartAddress + index*4) != CustomFlash::UnwrittenValue;
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
	for (unsigned int word = 0; word<CountWordsOfFlashString; word++ ) {
		*(uint32_t *)(FlashController::UICRStartAddress + OffsetToString + 4*word) = *(uint32_t*)(text + 4*word);
	}

	FlashController::disableWrite();
}
