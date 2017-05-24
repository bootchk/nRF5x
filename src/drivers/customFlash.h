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

/*
 * Index of words in UICR.
 *
 * Typically use word as a flag for certain exceptions and other events.
 * A flag is 'set' by writing all zeroes to it.
 * Events are specific to the application.
 */
enum FlagIndex {
	// index 0 not used
	HardFaultFlagIndex = 1,			// hw fault
	ExitFaultFlagIndex,				// assert() called
	BootedEventFlagIndex,
	EnterSyncLoopEventFlagIndex,
	ExcessPowerEventFlagIndex,		// Vcc above 3.6V
	BrownoutPowerEventFlagIndex,	// Vcc below 2.1V
	WorkEventFlagIndex,				// Worked e.g. flashed LED
	ExhaustedRadioPower,			// Vcc fell below 2.4V
	UnexpectedWake,					// sleep ended but timer not expired
	UnexpectedMsg,					// Radio IRQ while radio disabled?
	UnexpectedWakeWhileListen,		// radio on but woken for unknown reason
	StartSync,						// enough power to listen/send sync
	PauseSync,						// not enough power to listen/send sync
	LineNumberFlagIndex				// line no of assert
};



class CustomFlash {
	/*
	 * Offset in bytes to address in UICR we write a string. e.g. address 0x10000840
	 *
	 * Customer UICR is 0x80 (128) bytes (32 words)
	 * Reserve at least 20 words (80 bytes) for flags.
	 * Reserve 4 words (16 bytes) for string.
	 */
	static const int OffsetToString = 80;
public:


	// Write entire word to zero.
	static void writeZeroAtIndex(FlagIndex);

	// Write int to word
	static void writeIntAtIndex(FlagIndex, int);

	// Copy prefix of string to a fixed place in flash
	static void copyStringToFlash(const char* functionName);
};
