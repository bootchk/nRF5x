
#include <cassert>
#include <nrf_clock.h>	// HAL

/*
 * ISR for PowerClock device.
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
	 * - EVENT_
	 */

	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {

		// Signal wake reason to sleep
		didHFXOStartedInterruptFlag = true;

		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	}
	else {
		/*
		 * Brownout, write PC to flash.
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
	}
	// else Unexpected wake up? assert(false);
}

}	// extern C


