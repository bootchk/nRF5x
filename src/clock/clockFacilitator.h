#pragma once

#include "../types.h"



/*
 * Understands dependencies and startup times.
 * For higher level clocks:
 * - LongClock (extended, counting, LFClock)
 * - HfClock (just an oscillator, no counting)
 *
 *
 * LongClock and Timers depend on RTC0 (Counter and CompareRegisters) depends on LowFrequencyClock.
 * Starting requires IRQ enabled.
 *
 * Radio requires HfClock.
 * I am not sure how Radio behaves if HfClock is not stable.
 */
class ClockFacilitator {
public:
	/*
	 * Start LongClock and insure it is running so Timers can be started.
	 * Blocks.
	 * Varied duration.
	 */
	static void startLongClockWithSleepUntilRunning();

	/*
	 * Start HfClock.
	 * Sleep a constant time that is expected to be
	 * enough time for HFXO to be running stably.
	 */
	static void startHFClockWithSleepConstantExpectedDelay(OSTime delay);


	/*
	 * Start HfClock.
	 * Sleep a varying time, until conditoin: HFXO running stably.
	 * Sleep during startup to avoid energy consumption.
	 * Does not return until running, but is low power until then.
	 * HfCrystalClock takes about 1.2, 0.36 mSec to run stable depending on board design.
	 */
	static void startHFXOAndSleepUntilRunning();
};