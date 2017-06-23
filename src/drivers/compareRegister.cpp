
#include <cassert>

#include "compareRegister.h"

// Use HAL
#include "nrf_rtc.h"

/*
 * Implementation notes:
 *
 * See Nordic docs: enable event "enables routing to the PPI", and RTC task/event circuit is atypical from other peripherals.
 * Events occur even if "event routing" is disabled.
 * An event is a separate address.
 */



void CompareRegister::enableInterrupt() const {
	// Clear event so we don't interrupt immediately
	clearEvent();

	// Not needed: nrf_rtc_event_enable(NRF_RTC0, eventMask);
	nrf_rtc_int_enable(NRF_RTC0, interruptMask);
	// not ensure nvic enabled
}


void CompareRegister::disableInterrupt() const{
	// Not needed: nrf_rtc_event_disable(NRF_RTC0, eventMask);
	nrf_rtc_int_disable(NRF_RTC0, interruptMask);
	// not ensure nvic disabled
}


void CompareRegister::clearEvent() const{
	nrf_rtc_event_clear(NRF_RTC0, eventAddress);
}


bool CompareRegister::isEvent() const {
	return nrf_rtc_event_pending(NRF_RTC0, eventAddress);
}

void CompareRegister::set(const OSTime newCompareValue) const {
	/*
	 * require newCompareValue in range (checked earlier)
	 *
	 * HW might not generate event if you set compare to less than current count + 2
	 * But that is a concern above, here we just put it in the HW register
	 */

	nrf_rtc_cc_set(NRF_RTC0, selfIndex, newCompareValue);

	/*
	 * Ensures:
	 * Only that the compare register is some value.
	 * The HW does NOT guarantee that an event will be generated, when counter is already near the newCompareValue
	 */
}


