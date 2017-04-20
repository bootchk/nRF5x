#pragma once


/*
 * Custom non-volatile data.
 *
 * Data is erasable in a batch using a program such as nfjprog.
 * Erased data is all FFFF.
 * Writing can only clear bits.
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
	// Write entire word to zero.
	static void writeZeroAtIndex(unsigned int);
};
