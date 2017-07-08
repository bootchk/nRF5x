
#include "counter.h"

// Implementation: using HAL because it accounts for Cortex M4 write buffer using compiler flag  __CORTEX_M == 0x04
#include "nrf_rtc.h"	// HAL
#include "nvic.h"



void Counter::start(){
	// Start (power on and begin counting ticks from clock source)
	nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_START);
	/*
	 * Exists a delay until first increment.
	 * A minimum of 47 uSec when LF clock is already stable running.
	 * When LF clock is not already stable running, more delay.
	 * See CLOCK>LFCLK>LFXO on LFRC electrical specifications
	 *
	 * Here, we don't wait: no guarantee that first counter increment has come.
	 * We don't care because we only incur the delay once, at app startup,
	 * and it does not affect any critical timing.
	 *
	 * TODO, use TRIGOVRFLW to ensure first tick has come.
	 */
}

void Counter::stop(){
	nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_STOP);
}

/*
 * Don't need this:
 * enableEventRouting()
 * // Writes 1 to bit of EVTEN reg.  Does not affect other enabled events.
	nrf_rtc_event_enable(NRF_RTC0, RTC_EVTEN_OVRFLW_Msk);
 */
void Counter::configureOverflowInterrupt(){

	/*
	 * On a task/event architecture, device generates event which generates interrupt.
	 * Both event and/or interrupt can be enabled.
	 *
	 * !!! Interrupt must be enabled in device AND in nvic
	 */

	// Writes 1 to bit of INTENSET reg.  Does not affect other enabled interrupts.
	nrf_rtc_int_enable(NRF_RTC0, NRF_RTC_INT_OVERFLOW_MASK);

	Nvic::enableRTC0IRQ();

	// Interrupt can come at any time.
	// Usually, this is called shortly after starting Counter, so interrupt will come after period of Counter
}



void Counter::clearOverflowEventAndWaitUntilClear(){
	// HAL ensures that event is clear by reading the register after writing it (on Cortext M4)
	nrf_rtc_event_clear(NRF_RTC0, NRF_RTC_EVENT_OVERFLOW);
}

bool Counter::isOverflowEvent(){
	return nrf_rtc_event_pending(NRF_RTC0, NRF_RTC_EVENT_OVERFLOW);
}

OSTime Counter::ticks(){
	return nrf_rtc_counter_get(NRF_RTC0);
}

