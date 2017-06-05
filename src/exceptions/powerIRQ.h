
#include <cassert>
#include <nrf_clock.h>	// HAL
#include <nrf_power.h>	// HAL

#include "brownoutHandler.h"

/*
 * ISR for Power and Clock devices.
 *
 * Two functions:
 * - interrupt on HFClock running
 * - interrupt on POFCON detect brownout
 */


/*
 * This is included into longClockTimer.cpp
 * It doesn't seem to override the weak handler otherwise??
 */
//extern bool didHFXOStartedInterruptFlag;




// C so overrides weak handler
extern "C" {

void POWER_CLOCK_IRQHandler();

/*
 * If started event triggered event, set flag that sleeping loop reads.
 *
 *
 */
__attribute__ ((interrupt ("IRQ")))
void
POWER_CLOCK_IRQHandler() {

	/*
	 * POFCON and HFCLK use same IRQ vector.
	 * Interrupts enabled for two events:
	 * - EVENT_HFCLKSTARTED
	 * - EVENTS_POFWARN (Power Fail Warning)
	 *
	 * !!! It is conceivable the two events are concurrent, so check them both.
	 */

	// XXX if ! ( EVENT_HFCLKSTARTED || EVENTS_POFWARN ) assert programming bug
	// an event that we don't handle properly

	// Get fault address.
	// Specific to case where MSP is used (w/o RTOS)
	uint32_t* stackPointer = (uint32_t*) __get_MSP();
	uint32_t faultAddress = stackPointer[24/4]; // HW pushed PC onto stack 6 words into stack frame


	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {

		// Signal wake reason to sleep
		didHFXOStartedInterruptFlag = true;

		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	}
	if (nrf_power_event_check(NRF_POWER_EVENT_POFWARN)) {
		/*
		 * Brownout: write PC to flash so we can analyze later where in the app we brownout.
		 */
		brownoutWritePCToFlash(faultAddress);

		/*
		 * Typically little further execution is possible (power is failing)
		 * or we want to stop execution (at time of fault.)
		 */
		__asm("BKPT #0\n") ; // Break into the debugger, if it is running

		while(true) {};
	}
}

}	// extern C


