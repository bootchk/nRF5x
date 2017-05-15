
#include "compareRegister.h"

// Use HAL
#include "nrf_rtc.h"


void CompareRegister::init(
		nrf_rtc_event_t aEvent,
		unsigned int aEventMask,
		unsigned int aInterruptMask,
		unsigned int aIndex
		){
	/*
	 * Parameterize this instance with device addresses and masks.
	 */
	eventAddress = aEvent;
	eventMask = aEventMask;
	interruptMask = aInterruptMask;
	index = aIndex;
}


void CompareRegister::enableInterrupt(){
	// Clear event so we don't interrupt immediately
	clearEvent();

	nrf_rtc_event_enable(NRF_RTC0, eventMask);
	nrf_rtc_int_enable(NRF_RTC0, interruptMask);
	// not ensure nvic enabled
}


void CompareRegister::disableInterrupt(){
	nrf_rtc_event_disable(NRF_RTC0, eventMask);
	nrf_rtc_int_disable(NRF_RTC0, interruptMask);
	// not ensure nvic disabled
}


void CompareRegister::clearEvent(){
	nrf_rtc_event_clear(NRF_RTC0, eventAddress);
}


bool CompareRegister::isEvent(){
	return nrf_rtc_event_pending(NRF_RTC0, eventAddress);
}

void CompareRegister::set(unsigned int timeout){
	/*
	 * require timeout in range (checked earlier)
	 *
	 * Timeout might fail if you set compare to less than current count + 2
	 */
	// require event disabled?
	unsigned int currentCount = nrf_rtc_counter_get(NRF_RTC0);
	/*
	 * Modulo 24-bit math:
	 * value is computed in 32-bit math but writing to the register masks with 0xFFFFFF,
	 */
	nrf_rtc_cc_set(NRF_RTC0, index, currentCount + timeout);
	// assert nrf_rtc_cc_get() = (currentCount + timeout) % 0xFFFFFF
}


