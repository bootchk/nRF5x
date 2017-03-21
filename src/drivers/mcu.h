#pragma once

/*
 * "driver" for microcontroller device.
 * Hides target specifics.
 *
 * AKA system
 *
 *
 */
class MCU {
public:
	/* sleep mcu until an event */
	static void sleep();

	/*
	 * Flush ARM write cache to insure writes to IO memory mapped registers take effect now.
	 *
	 * Unless you call this, the write cache is not flushed until some arbitrary read from memory
	 * (which flushes cache to ensure what was written is what is read.)
	 *
	 * Note also that the peripheral bus is at a slower clock (16Mhz) on the NRF52
	 * so four instruction cycles may pass before a peripheral sees a write?
	 */
	static void flushWriteCache();
};
