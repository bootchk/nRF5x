

#include <cassert>
#include <nrf_clock.h>	// HAL
#include <nrf_power.h>	// HAL

#include "brownoutManager.h"
#include "faultHandlers.h"
#include "../modules/powerMonitor.h"
#include "../modules/sleeper.h"


/*
 * ISR for Power and Clock devices.
 *
 * Two functions:
 * - interrupt on HFClock running
 * - interrupt on POFCON detect brownout
 */

/*
 * Testing procedure for brownout detection/writing to flash:
 * Use a NRF52DK with custom board connected by SW and having separate variable power supply.
 * Set power supply to 3V.
 * Start debugger and execute a main that harnesses this code.
 * Lower the power supply to below 2.3V (or whatever you have set for brownout detection.)
 * The debugger should stop at the BKPT below.
 * Word 19 of UICR should contain the PC where the fault occurred.
 * Use Eclipse memory view to view UICR at 0x10001080, use unsigned int rendering to view fault address in decimal.
 */

/*
 * This file is included into hfClock.cpp
 * If the app uses hfClock, then the linker overwrites the weak handler with this one.
 */

extern BrownoutManager brownoutManager;



// C so overrides weak handler
extern "C" {

void POWER_CLOCK_IRQHandler();


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
	uint32_t faultAddress = stackPointer[12]; // HW pushed PC onto stack 6 words into stack frame


	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {
		// Signal wake reason to sleep
		Sleeper::setReasonForWake(ReasonForWake::HFClockStarted);

		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	}
	if (nrf_power_event_check(NRF_POWER_EVENT_POFWARN)) {
		/*
		 * Brownout: write PC to flash so we can analyze later where in the app we brownout.
		 */
		brownoutManager.recordToFlash(faultAddress);

		/*
		 * Typically little further execution is possible (power is failing).
		 *
		 * Alternatives:
		 * 1) stop execution (at time of fault.)
		 * 2) continue, signal to others BrownoutWarning, and wait for actual BOR
		 *
		 * 1) BKPT causes an additional hard fault on Cortext M0
		 * __asm("BKPT #0\n") ; // Break into the debugger, if it is running
		 * 1) resetOrHalt();
		 *   will enter infinite loop
		 */

		/*
		 *  2) Proceed and wait for actual BOR
		 */
		// Signal
		Sleeper::setReasonForWake(ReasonForWake::BrownoutWarning);

		// Disable further POFWARN events
		nrf_power_event_clear(NRF_POWER_EVENT_POFWARN);
		PowerMonitor::disableBrownoutDetection();

		// return from interrupt
	}
}

}	// extern C


