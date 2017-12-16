
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


#include "compareRegArray.h"	// externs the array of compareRegister
/*
 * The array of compare registers is exported.
 * How it is initialized here is platform dependent.
 *
 * !!!! CompareRegisters are constant (the facade is constant, the HW registers are of course writeable.)
 *
 * Parameters of compareRegisters are fixed by hw design of platform (defined by macros.)
 *
 * No need to init CompareRegister, they are constructed const.
 *
 * This does not guarantee the state of the hw compare registers
 * (but typically, all are in POR reset state i.e. disabled.)
 */
const CompareRegister compareRegisters[2] = {
		CompareRegister(NRF_RTC_EVENT_COMPARE_0, NRF_RTC_INT_COMPARE0_MASK, 0),
		CompareRegister(NRF_RTC_EVENT_COMPARE_1, NRF_RTC_INT_COMPARE1_MASK, 1)
};





void CompareRegister::enableInterrupt() const {
	// Clear event so we don't interrupt immediately
	clearEvent();

	// Not needed: nrf_rtc_event_enable(NRF_RTC1, eventMask);
	nrf_rtc_int_enable(NRF_RTC1, interruptMask);
	// not ensure nvic enabled
}

/*
 * This is not necessarily within an ISR (with interrupts enabled.)
 * An interrupt may occur in the middle of this.
 * Thus the correct order is disableInterrupt, then clearEvent.
 */
void CompareRegister::disableInterruptAndClearEvent() const{
	disableInterrupt();
	clearEvent();
	/*
	 * Ensures event is clear.
	 * !!! Does not ensure that interrupt did not occur and signal sent during call here.
	 * E.G. reasonForWake could be set.
	 */
}


void CompareRegister::disableInterrupt() const{
	// Not needed: nrf_rtc_event_disable(NRF_RTC1, eventMask);
	nrf_rtc_int_disable(NRF_RTC1, interruptMask);
	// not ensure nvic disabled
}


void CompareRegister::clearEvent() const{
	// cast platform independent type into platform specific type
	nrf_rtc_event_clear(NRF_RTC1, (nrf_rtc_event_t) eventAddress);
}


bool CompareRegister::isEvent() const {
	return nrf_rtc_event_pending(NRF_RTC1, (nrf_rtc_event_t) eventAddress);
}

void CompareRegister::set(const uint32_t newCompareValue) const {
	/*
	 * require newCompareValue in range (checked earlier)
	 *
	 * HW might not generate event if you set compare to less than current count + 2
	 * But that is a concern above, here we just put it in the HW register
	 */

	nrf_rtc_cc_set(NRF_RTC1, selfIndex, newCompareValue);

	/*
	 * Ensures:
	 * Only that the compare register is some value.
	 * The HW does NOT guarantee that an event will be generated, when counter is already near the newCompareValue
	 */
}


