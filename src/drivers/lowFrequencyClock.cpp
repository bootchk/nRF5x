#include <cassert>
#include <inttypes.h>
// The following include includes sdk_config.h
//#include "nrf_drv_clock.h"
#include "nrf_clock.h"

#include "lowFrequencyClock.h"



// Uses HAL instead of nrf_drv_clock


/*
 * Start.
 * !!! Does not guarantee clock is ready: use isStarted();
 */
void LowFrequencyClock::start() {
	/*
	 * Assumes source configured prior, else default source.
	 */
	nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
	// We leave the started event set, it means little here and interrupt is not enabled

	// Wait for event
	while (!isStarted()) {}

	// not assert isRunning()
}

bool LowFrequencyClock::isStarted() {
	return nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED);
}

/*
 * Is the lf clock running (and stable).
 * More that just "is started", but is the hw beyond its startup delay.
 * Note this is independent of clock source.
 * This does NOT guarantee the source.
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
	 * Source cannot be configured while "running" (Nordic docs.)
	 * I assume they mean "started."
	 */
	if (isStarted()) {
		assert(false);
	}
	nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
}
