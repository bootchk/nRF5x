
// Use HAL
#include <nrf_rtc.h>

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
	const nrf_rtc_event_t eventAddress; // HW address of event register
	const nrf_rtc_int_t interruptMask; // Mask for bit in HW interrupt register
	const unsigned int selfIndex = 0;

public:
	CompareRegister(
			const nrf_rtc_event_t aEventAddress,
			const nrf_rtc_int_t aInterruptMask,
			const unsigned int aIndex
	):
		// Initializer list required for const data members, parameterized
		eventAddress(aEventAddress),
		interruptMask(aInterruptMask),
		selfIndex(aIndex) {}	// body required


	void enableInterrupt() const;
	void disableInterrupt() const;
	void clearEvent() const;
	bool isEvent() const;

	/*
	 * Set value.
	 * The value is an "alarm time" on the circular Counter clock.
	 * Whose type is OSTime (24-bit)
	 */
	void set(const OSTime newCompareValue) const;
};
