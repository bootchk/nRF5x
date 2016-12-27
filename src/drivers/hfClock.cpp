#include <cassert>

#include <nrf.h>	// direct access to NRF_CLOCK
#include <nrf_clock.h>	// HAL

#include "hfClock.h"


namespace {

bool didInterruptStartingEvent = false;

void enableInterruptOnRunning() {
	// Event signals clock is running !!!!
	nrf_clock_int_enable(NRF_CLOCK_INT_HF_STARTED_MASK);
}
void disableInterruptOnRunning() {
	nrf_clock_int_disable(NRF_CLOCK_INT_HF_STARTED_MASK);
}



extern "C" {

/*
 * If started event triggered event, set flag that sleeping loop reads.
 *
 * C so overrides default
 */
void POWER_CLOCK_IRQHandler() {
	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {
		didInterruptStartingEvent = true;
		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	}
}

}	// extern C

}  // namespace



/*
 * Read carefully Nordic docs.
 *
 * The hf clock has two sources:
 * - internal RC oscillator (or PLL?):  HFINT
 * - external crystal:   HXFO
 * The HXFO is more accurate (smaller ppm error.)
 * The HXFO uses more power??? so typically the HFINT is used.
 *
 * The hf clock is automatically started when the mcu is on and not sleeping,
 * because the hf clock provides the clock for the mcu.
 * The hf clock enters a power saving mode when the mcu (or other peripherals) don't need it.
 *
 * You control the source by enabling task HFCLKSTARTED and HFCLKSTOP.
 * !!! These specifically start the HXFO !!!
 * You can read the source in HFCLKSTAT (but it is not writeable.)
 *
 * The radio requires HXFO (to precisely delimit bits on the carrier?)
 */

/*
 * Only trigger start task.
 */
void HfCrystalClock::start(){

	// Enable the High Frequency clock to the system as a whole
	nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);

#ifdef OLD
	non-HAL
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	// No need to flush ARM write cache, the read below will do it
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
#endif
}




bool HfCrystalClock::isRunning(){
	/*
	 * This doesn't use Hal and is wrong: just indicates event.
	 * while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { }
	 */
	// cast to avoid warning, poor SDK
	return nrf_clock_hf_is_running( (nrf_clock_hfclk_t) CLOCK_HFCLKSTAT_SRC_Xtal);
}


void HfCrystalClock::startAndSleepUntilRunning() {
	didInterruptStartingEvent = false;
	assert(!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED));
	enableInterruptOnRunning();
	start();
	// sleep until IRQ for started event signals
	while (! didInterruptStartingEvent) {
		/*
		 * !!! Event must come, and we ignore other events from clock and other sources
		 * e.g. timers for flashing LED's (whose interrupts will occur, but returns to this thread.)
		 */
		// do nothing
	}
	// assert event is cleared.

	// Really don't need to disable interrupt.
	disableInterruptOnRunning();

	assert(isRunning());
}


void HfCrystalClock::startAndWaitUntilRunning(){

	// Enable the High Frequency clock to the system as a whole
	nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
#ifdef  OLD
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	// No need to flush ARM write cache, the read below will do it
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
#endif

	// Spin (wasting energy)
	while ( !isRunning()  ) {}

	// FUTURE use nrf_driver/hal functions
	assert(NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Msk);	// 1 == Xtal
	assert(NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKRUN_STATUS_Msk);	// 1 == running
}


void HfCrystalClock::stop(){
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
	// non-HAL NRF_CLOCK->TASKS_HFCLKSTOP = 1;

	/*
	 *  Will generate event, but interrupt not enabled, and we don't care whether event really happens:
	 *  not spinning for event indicating stopped
	 */
}

