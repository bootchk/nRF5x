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
 */

/*
 * Index of words in UICR.
 *
 * Typically use word as a flag for certain exceptions and other events.
 * A flag is 'set' by writing all zeroes to it.
 *
 * Exceptions are handled generically, writing PC to flash.
 * Flagged events are specific to the application.
 */
enum FlagIndex {
	// First three are Program Counters of exceptions or Line Number, not flags
	HardFaultPCIndex = 0,			// hw fault PC
	BrownoutPCIndex,				// brownout PC
	BrownoutCallbackIndex,			// phase of algorithm
	LineNumberIndex, 				// line no of assert

	// Flags. Normal algorithm stepping is captured in Phase.
	/*
	 * Bugs caught by code for robustness.
	 */
	ExitFlagIndex,					// 4. undistinguished exit
	UnexpectedWake,					// sleep ended but timer not expired
	UnexpectedMsg,					// Radio IRQ while radio disabled?
	UnexpectedWakeWhileListen,		// radio on but woken for unknown reason
	OverSlept,						// 8.
	/*
	 * Rare but expected events.
	 */
	ExcessPowerEventFlagIndex,		// Vcc above 3.6V
	WorkEventFlagIndex,				// Worked e.g. flashed LED
	NoPowerToFish,					// Vcc fell below 2.5V
	NoPowerToStartSyncSlot,			// 12. "
	NoPowerToHalfSyncSlot,			//  "
	PauseSync,					    // not enough power to listen/send sync

	BrownoutCallback2Index,			// reason for wake
	BrownoutCallback3Index,			// time of sleep
};



class CustomFlash {
	/*
	 * Offset in bytes to address in UICR we write a string. e.g. address 0x10000840
	 *
	 * Customer UICR is 0x80 (128) bytes (32 words)
	 * Reserve at least 20 words (80 bytes) for flags.
	 * Reserve 12 words (48 bytes) for string.
	 */
	static const int OffsetToString = 80;
	static const unsigned int CountWordsOfFlashString = 12;
	static const unsigned int UnwrittenValue = 0xFfFfFfFf;

public:


	// Write entire word to zero.
	static void writeZeroAtIndex(FlagIndex);

	// Write int to word if not already written
	static void tryWriteIntAtIndex(FlagIndex, unsigned int);

	// Did we already write to a word?
	static bool isWrittenAtIndex(FlagIndex);

	// To a fixed place in flash
	static void copyStringPrefixToFlash(const char* functionName);
};
