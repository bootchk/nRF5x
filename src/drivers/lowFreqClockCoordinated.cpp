#include <drivers/lowFreqClockCoordinated.h>
#include <cassert>
#include <inttypes.h>

/*
 * To be coordinated, use nrf_drv_clock instead of hal/nrf_clock.h
 * nrf_drv_clock.h includes sdk_config.h
 * nrf_drv_clock is in components/drivers_nrf/clock
 * Also required path to components/drivers_nrf/common
 *
 * nrf_clock functions are visible, be careful not to use them
 */
#include "nrf_drv_clock.h"





namespace {

	// flag meaning only that start() was called
	// bool _isStarted = false;

	Callback lfClockStartedCallback = nullptr;
	Callback hfClockStartedCallback = nullptr;

	/*
	 * Node in chain of handlers (callbacks)
	 */
	nrf_drv_clock_event_handler_t node = {
			p_next: nullptr,
			event_handler: callback
	};

}


void LowFreqClockCoordinated::registerCallbacks(Callback lfStarted, Callback hfStarted) {
	lfClockStartedCallback = lfStarted;
	hfClockStartedCallback = hfStarted;
}


/*
 * MULTIPROTOCOL must use coordinate with SD, can't use POWERCLOCK_ISR
 */
#ifndef MULTIPROTOCOL
void LowFreqClockCoordinated::clockISR(){
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

#else

/*
 * Get callback from module with started event.
 */

void callback() {

}

#endif


/*
 * Implementation notes:
 * !!! Start() does not guarantee clock is running!!!
 * But according to the documents, the clock initially begins ticking on the RC oscillator,
 * and switches to the XTAL oscillator automatically.
 * Time for RC to start is 600uSec (say a few thousand instructions on the nRF52.)
 * Timer for XTAL to start is 0.25 seconds !!!
 */


void LowFreqClockCoordinated::init() {
	ret_code_t err_code;

	err_code = nrf_drv_clock_init();
	APP_ERROR_CHECK(err_code);
	// TODO ignore return NRF_ERROR_MODULE_ALREADY_INITIALIZED
}



void LowFreqClockCoordinated::start() {
	/*
	 * Clock source is configured by ???
	 */
	nrf_drv_clock_lfclk_request(nrf_drv_clock_handler_item_t * callback);

	/*
	 * !!! Not wait for any event.
	 * not assert isRunning()
	 * See Nordic docs re callback on started.
	 */
}


bool LowFreqClockCoordinated::isStarted() {
	return _isStarted;
}

bool LowFreqClockCoordinated::isStartedEvent() {
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
bool LowFreqClockCoordinated::isRunning() {
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
void LowFreqClockCoordinated::configureXtalSource() {

	/*
	 * Require: not started.
	 * Nordic docs say "Source cannot be configured while running."  I assume they mean "started."
	 */
	assert( ! isRunning());

	nrf_clock_lf_src_set(NRF_CLOCK_LFCLK_Xtal);
}


void LowFreqClockCoordinated::enableInterruptOnStarted(){ nrf_clock_int_enable(NRF_CLOCK_INT_LF_STARTED_MASK); }
void LowFreqClockCoordinated::disableInterruptOnStarted(){ nrf_clock_int_disable(NRF_CLOCK_INT_LF_STARTED_MASK); }


void LowFreqClockCoordinated::spinUntilRunning() {
	while( !isRunning()) {}
}
