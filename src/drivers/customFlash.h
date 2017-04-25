#pragma once


/*
 * Custom non-volatile data.
 * This knows how to write data to flash (persists, non-volatile.)
 *
 * Data is erasable in a batch using a program such as nfjprog.
 * Erased data is all FFFF.
 * Writing can only clear bits.
 * !!! Thus these functions only work once (until you erase flash again.)
 *
 * In Nordic UICR
 *
 * UICR is distinctive flash: eraseable separately from other flash.
 * Alternative is other flash memory.
 * Note retained registers are retained through SYSTEM_OFF, but not through reset.
 *
 *
 */
class CustomFlash {
public:
	// Where in UICR we write a string
	static const int OffsetToString = 20;

	// Write entire word to zero.
	static void writeZeroAtIndex(unsigned int);

	// Write int to word
	static void writeIntAtIndex(unsigned int, int);

	// To a fixed place in flash
	static void copyStringToFlash(const char* functionName);
};
