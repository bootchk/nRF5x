
#include <cassert>
#include <nrf_clock.h>	// HAL
#include <nrf_power.h>	// HAL

/*
 * ISR for Power and Clock devices.
 *
 * Two functions:
 * - interrupt on HFClock running
 * - interrupt on POFCON detect brownout
 */


extern bool didHFXOStartedInterruptFlag;




// C so overrides weak handler
extern "C" {

void POWER_CLOCK_IRQHandler();

/*
 * If started event triggered event, set flag that sleeping loop reads.
 *
 *
 */
__attribute__ ((interrupt ("POWER_CLOCK_IRQ")))
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

	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {

		// Signal wake reason to sleep
		didHFXOStartedInterruptFlag = true;

		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	}
	if (nrf_power_event_check(NRF_POWER_EVENT_POFWARN)) {
		/*
		 * Brownout, write PC to flash.
		 * So we can analyze later where in the app we brownout.
		 *
		 * !!! If the system repeatedly brownouts and reboots,
		 * the PC is only written to flash once
		 * since a second write to flash garbles the first write.
		 */
		__asm(  ".syntax unified\n"
		        "MOVS   R0, #4  // test mode\n"
		        "MOV    R1, LR  \n"
		        "TST    R0, R1  \n"
		        "BEQ    _MSP    \n"
		        "MRS    R0, PSP // load r0 with MainSP\n"
		        "bl      ExceptionHandlerWritePCToFlash      \n"
		        "_MSP:  \n"
		        "MRS    R0, MSP // load r0 with ProcessSP\n"
		        "bl      ExceptionHandlerWritePCToFlash      \n"
		        );
		// Never returns
	}
}

}	// extern C


