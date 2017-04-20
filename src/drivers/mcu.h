#pragma once

/*
 * "driver" for microcontroller device, i.e. the ARM cpu as distinguished from independent peripherals.
 * Hides target specifics.
 *
 * All functions are class methods: no need to define an instance.
 */
class MCU {
public:

	/* sleep mcu until an event */
	static void sleep();

	/*
	 * Flush ARM write cache to insure writes to IO memory mapped registers take effect now.
	 * AKA write buffers between cpu and peripheral bus, see ARM docs.
	 *
	 * Unless you call this, the write cache is not flushed until some arbitrary read from memory
	 * (which flushes cache to ensure what was written is what is read.)
	 *
	 * Note also that the peripheral bus is at a slower clock (16Mhz) on the NRF52
	 * so four instruction cycles may pass before a peripheral sees a write?
	 */
	static void flushWriteCache();

	/*
	 * The instruction cache takes more power but might reduce cpu active time.
	 * IOW, it is not clear that enabling instruction cache saves overall power.
	 */
	static void enableInstructionCache();
};
