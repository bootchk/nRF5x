#pragma once

/*
 * "driver" for microcontroller device, i.e. the ARM cpu as distinguished from independent peripherals.
 * Hides target specifics.
 *
 * All functions are class methods: no need to define an instance.
 */
class MCU {
public:

	/*
	 * sleep mcu until an event
	 *
	 * Note used several places:
	 * - hfxo startup
	 * - sleeper
	 */
	static void sleep();

	// Clear mcu internal event flag
	static void clearEventRegister();



	/*
	 * Flush ARM write cache to insure writes to IO memory mapped registers take effect now.
	 * AKA write buffers between cpu and peripheral bus, see ARM docs.
	 *
	 * Unless you call this, the write cache is not flushed until some arbitrary read from memory
	 * (which flushes cache to ensure what was written is what is read.)
	 *
	 * Note also that the peripheral bus is at a slower clock (16Mhz) on the NRF52_SERIES
	 * so four instruction cycles may pass before a peripheral sees a write?
	 */
	static void flushWriteCache();

	/*
	 * The instruction cache takes more power but might reduce cpu active time.
	 * IOW, it is not clear that enabling instruction cache saves overall power.
	 */
	static void enableInstructionCache();

	// There is no enableIRQ(), at reset they are enabled
	// Disable all interrupts (using mcu PRIMASK register.)
	// Since there is no enableIRQ(), this should only be used in a fatal condition
	static void disableIRQ();

	static bool isResetReason();
	static void clearResetReason();

	/*
	 * Only effective for NRF52_SERIES
	 */
	static bool isDebugMode();

	/*
	 * If ARM debug mode is enabled, enter Debug mode, else generate hardfault.
	 * If a debugging probe is attached, the probe enables debug mode and controls the app,
	 * and in that case, this will cause gdb to show the location in its GUI.
	 */
	static void breakIntoDebuggerOrHardfault();

};
