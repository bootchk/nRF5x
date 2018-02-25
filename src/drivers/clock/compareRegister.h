
#pragma once

/*
 * Types of initializer parameters are platform independent types so API is platform independent.
 * Implementation uses platform specific types
 */

#include <stddef.h>		// size_t

#include "counter.h"	// OSTime type on Counter

/*
 * Thin wrapper/facade on HW compare register of Nordic RTC device.
 *
 * Not a singleton; has non-static methods.
 *
 * But instances are constants and methods are const functions:
 * they change the hardware but no data members.
 */

class CompareRegister {

	// We are not enabling event routing:  unsigned int eventMask = RTC_EVTEN_COMPARE0_Msk;
	const size_t eventAddress;	// nrf_rtc_event_t  HW offset of event register from device base address
	const uint32_t eventMask; // nrf_rtc_int_t  Mask for bit in HW interrupt registers INTEN or EVTEN !!!
	const unsigned int selfIndex = 0;

public:
	CompareRegister(
			const size_t aEventAddress,	// nrf_rtc_event_t
			const uint32_t aEventMask,	// nrf_rtc_int_t
			const unsigned int aIndex
	):
		// Initializer list required for const data members, parameterized
		eventAddress(aEventAddress),
		eventMask(aEventMask),
		selfIndex(aIndex) {}	// body required



	void enableInterrupt() const;

	/*
	 * Enable/disable "event routing" i.e. generation of signal (pulse on event) to PPI
	 *
	 * The counter rolls over, and compare match will generate the event repetitively, unless disabled.
	 */
	void enableEventSignal() const;
	void disableEventSignal() const;


private:
	void disableInterrupt() const;
	void clearEvent() const;

public:
	void disableInterruptAndClearEvent() const;
	bool isEvent() const;

	/*
	 * Set value.
	 * The value is an "alarm time" on the circular Counter clock.
	 * Whose type is OSTime (24-bit)
	 */
	void set(const uint32_t newCompareValue) const;
};
