#pragma once


/*
 * Understands dependencies and startup times.
 *
 * LongClock and Timers depend on RTC0 (Counter and CompareRegisters) depends on LowFrequencyClock.
 */
class ClockFacilitator {
public:
	/*
	 * Start LongClock and insure it is running so Timers can be started.
	 */
	static void startLongClockWithSleepUntilRunning();
};
