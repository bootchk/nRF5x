
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
	const size_t eventAddress;	// nrf_rtc_event_t eventAddress; // HW offset of event register from device base address
	const uint32_t interruptMask; // nrf_rtc_int_t interruptMask; // Mask for bit in HW interrupt register
	const unsigned int selfIndex = 0;

public:
	CompareRegister(
			const size_t aEventAddress,	// nrf_rtc_event_t aEventAddress,
			const uint32_t aInterruptMask,	// nrf_rtc_int_t aInterruptMask,
			const unsigned int aIndex
	):
		// Initializer list required for const data members, parameterized
		eventAddress(aEventAddress),
		interruptMask(aInterruptMask),
		selfIndex(aIndex) {}	// body required


	void enableInterrupt() const;

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
