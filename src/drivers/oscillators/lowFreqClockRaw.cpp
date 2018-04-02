
#include <cassert>
#include <inttypes.h>

#include "lowFreqClockRaw.h"

// Uses HAL instead of nrf_drv_clock
#include "nrf_clock.h"	// hal, i.e. raw



namespace {

	// flag meaning only that start() was called
	bool _isStarted = false;

	VoidCallback lfClockStartedCallback = nullptr;
	VoidCallback hfClockStartedCallback = nullptr;

}


void LowFreqClockRaw::registerCallbacks(VoidCallback lfStarted, VoidCallback hfStarted) {
	lfClockStartedCallback = lfStarted;
	hfClockStartedCallback = hfStarted;
}



void LowFreqClockRaw::clockISR(){
	/*
	 * !!! Order is important.
	 * Because we leave EVENT_LFCLCKSTARTED set, this code will set reasonForWake always.
	 * So put other reasons (HFCLKCSTARTED) after it.
	 */
	// TODO prioritize reasonsforwake
	/*
	 * LF
	 */
	if (nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED)) {
		// Signal
		assert(lfClockStartedCallback);
		lfClockStartedCallback();


		/*
		 * !!!
		 * The event does NOT need to remain set so that the RTC Counter continues to increment.  See Errata 20.
		 * Disable interrupt and clear the event.
		 * Assert we only start the LFCLK once, when no other interrupts are enabled.
		 */
		disableInterruptOnStarted();
		nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
	}

	/*
	 * HF
	 */
	if (nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED)) {
		assert(hfClockStartedCallback);
		hfClockStartedCallback();

		// Clear event so interrupt not triggered again.
		nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
		// Interrupt remains enabled because we start and stop HF clock often.
	}
}


/*
 * Not needed (and dangerous)
 * clearLFClockStartedEvent()
 * nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
 */



/*
 * Implementation notes:
 * !!! Start() does not guarantee clock is running!!!
 * But according to the documents, the clock initially begins ticking on the RC oscillator,
 * and switches to the XTAL oscillator automatically.
 * Time for RC to start is 600uSec (say a few thousand instructions on the nRF52.)
 * Timer for XTAL to start is 0.25 seconds !!!
 */


void LowFreqClockRaw::start() {
	/*
	 * Assumes source configured prior, else default source.
	 */
	nrf_clock_event_clear(NRF_CLOCK_EVENT_LFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_LFCLKSTART);
	// We leave the started event set, it means little here and interrupt is not enabled

	_isStarted = true;

	/*
	 * !!! Not wait for any event.
	 * not assert isRunning()
	 */
}


bool LowFreqClockRaw::isStarted() {
	return _isStarted;
}

bool LowFreqClockRaw::isStartedEvent() {
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
bool LowFreqClockRaw::isRunning() {
	/*
	 * !!! It is not correct to check started event:
	 * return nrf_clock_event_check(NRF_CLOCK_EVENT_LFCLKSTARTED);
	 * See components/drivers_nrf/hal/nrf_clock.h where is is implemented as checking the hw clock status register.
	 */
	return nrf_clock_lf_is_running();
}


/*
 * Configure to high accuracy xtal clock source.
 * If the board has no xtal???
 * If you don't call this, source is reset default of LFRC.
 */
void LowFreqClockRaw::configureXtalSource() {

	/*
	 * Require: not started.
	 * Nordic docs say "Source cannot be configured while running."  I assume they mean "started."
	 */
	assert( ! isRunning());

	nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
}


void LowFreqClockRaw::enableInterruptOnStarted(){ nrf_clock_int_enable(NRF_CLOCK_INT_LF_STARTED_MASK); }
void LowFreqClockRaw::disableInterruptOnStarted(){ nrf_clock_int_disable(NRF_CLOCK_INT_LF_STARTED_MASK); }


void LowFreqClockRaw::spinUntilRunning() {
	while( !isRunning()) {}
}
