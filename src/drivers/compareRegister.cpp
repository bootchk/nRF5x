
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

void CompareRegister::init(
		nrf_rtc_event_t aEvent,
		//unsigned int aEventMask,
		nrf_rtc_int_t   aInterruptMask,
		unsigned int aIndex
		){
	/*
	 * Parameterize this instance with device addresses and masks.
	 */
	eventAddress = aEvent;
	//eventMask = aEventMask;
	interruptMask = aInterruptMask;
	selfIndex = aIndex;
}


void CompareRegister::enableInterrupt(){
	// Clear event so we don't interrupt immediately
	clearEvent();

	// Not needed: nrf_rtc_event_enable(NRF_RTC0, eventMask);
	nrf_rtc_int_enable(NRF_RTC0, interruptMask);
	// not ensure nvic enabled
}


void CompareRegister::disableInterrupt(){
	// Not needed: nrf_rtc_event_disable(NRF_RTC0, eventMask);
	nrf_rtc_int_disable(NRF_RTC0, interruptMask);
	// not ensure nvic disabled
}


void CompareRegister::clearEvent(){
	nrf_rtc_event_clear(NRF_RTC0, eventAddress);
}


bool CompareRegister::isEvent(){
	return nrf_rtc_event_pending(NRF_RTC0, eventAddress);
}

void CompareRegister::set(unsigned int newCounterValue){
	/*
	 * require newCounterValue in range (checked earlier)
	 *
	 * HW might not generate event if you set compare to less than current count + 2
	 * But that is a concern above.
	 * Here we just put it in the HW register
	 */

	nrf_rtc_cc_set(NRF_RTC0, selfIndex, newCounterValue);

	/*
	 * Ensures:
	 * Only that the compare register is some value.
	 * If the counter value when we return is greater than the counter value when called + timeout,
	 * an event was generated.
	 * And a subsequent interrupt enable will generate an interrupt.
	 */
}


