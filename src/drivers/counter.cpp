
#include "counter.h"

// Implementation:
#include "nrf_rtc.h"	// HAL
#include "nvic.h"



void Counter::start(){
	// Start (power on)
	nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_START);
	/*
	 * Exists a delay until first increment.
	 * Delay due to oscillator startup.
	 * Delay depends on source oscillator.
	 * Delay for LFXO is 0.25 seconds.
	 * Delay for LFRC is 0.6 msec
	 * See CLOCK>LFCLK>LFXO on LFRC electrical specifications
	 *
	 * Here, we don't wait: no guarantee that first tick has come.
	 *
	 * TODO, use TRIGOVRFLW to ensure first tick has come.
	 */
}

void Counter::stop(){
	nrf_rtc_task_trigger(NRF_RTC0, NRF_RTC_TASK_STOP);
}


void Counter::configureOverflowInterrupt(){

	/*
	 * On a task/event architecture, device generates event which generates interrupt.
	 * Both event and/or interrupt can be enabled.
	 *
	 * !!! Interrupt must be enabled in device AND in nvic
	 */

	// Writes 1 to bit of EVTEN reg.  Does not affect other enabled events.
	nrf_rtc_event_enable(NRF_RTC0, RTC_EVTEN_OVRFLW_Msk);

	// Writes 1 to bit of INTENSET reg.  Does not affect other enabled interrupts.
	nrf_rtc_int_enable(NRF_RTC0, NRF_RTC_INT_OVERFLOW_MASK);

	Nvic::enableRTC0IRQ();

	// Interrupt can come at any time.
	// Usually, this is called shortly after starting Counter, so interrupt will come after period of Counter
}

#if __CORTEX_M != 0x04
#error "Foo"
#endif

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

