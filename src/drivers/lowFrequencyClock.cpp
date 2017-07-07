#include <cassert>
#include <inttypes.h>
// The following include includes sdk_config.h
//#include "nrf_drv_clock.h"
#include "nrf_clock.h"

#include "lowFrequencyClock.h"



// Uses HAL instead of nrf_drv_clock


/*
 * Implementation notes:
 * !!! Start() does not guarantee clock is running!!!
 * But according to the documents, the clock initially begins ticking on the RC oscillator,
 * and switches to the XTAL oscillator automatically.
 * Time for RC to start is 600uSec (say a few thousand instructions on the nRF52.)
 * Timer for XTAL to start is 0.25 seconds !!!
 */


void LowFrequencyClock::start() {
	/*
	 * Assumes source configured prior, else default source.
	 */
	nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
	// We leave the started event set, it means little here and interrupt is not enabled

	/*
	 * !!! Not wait for any event.
	 * not assert isRunning()
	 */
}

#ifdef OBSOLETE

// Wait for event
	while (!isStarted()) {}
#endif

bool LowFrequencyClock::isStartedEvent() {
	return nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED);
}


/*
 * Is the lf clock running (and stable).
 * More that just "is started", but is the hw beyond its startup delay.
 *
 * Startup delay is:
 *   RC:  600uSec
 *   XO:  0.25Sec
 *
 * !!! Note this is independent of clock source.  This does NOT guarantee the source.
 */
bool LowFrequencyClock::isRunning() {
	/*
	 * !!! It is not correct to check started event.
	 * return nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED);
	 */
	return nrf_clock_lf_is_running();
}


/*
 * Configure to high accuracy xtal clock source.
 * If the board has no xtal???
 * If you don't call this, source is reset default of LFRC.
 */
void LowFrequencyClock::configureXtalSource() {

	/*
	 * Require: not started.
	 * Nordic docs say "Source cannot be configured while running."  I assume they mean "started."
	 */
	assert( ! isRunning());

	nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
}


void LowFrequencyClock::enableInterruptOnStarted(){ nrf_clock_int_enable(NRF_CLOCK_INT_LF_STARTED_MASK); }
void LowFrequencyClock::disableInterruptOnStarted(){ nrf_clock_int_disable(NRF_CLOCK_INT_LF_STARTED_MASK); }


void LowFrequencyClock::spinUntilRunning() {
	while( !isRunning()) {}
}
