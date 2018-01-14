#include "lowFreqClockCoordinated.h"
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
	bool _isStarted = false;

}


#ifdef NOT_USED

	Callback lfClockStartedCallback = nullptr;
	Callback hfClockStartedCallback = nullptr;

	/*
	 * Node in chain of handlers (callbacks)
	 */
	nrf_drv_clock_event_handler_t handlerChainNode;

	/*
	= {
			p_next: nullptr,
			event_handler: callback
	};
	 */

	/*
	 * Handler of type nrf_drv_clock_event_handler_t == void (*)(nrf_drv_clock_evt_type_t event);
	 */
	void startedCallback(nrf_drv_clock_evt_type_t event) {

	}

void LowFreqClockCoordinated::registerCallbacks(Callback lfStarted, Callback hfStarted) {
	lfClockStartedCallback = lfStarted;
	hfClockStartedCallback = hfStarted;
}


/*
 * MULTIPROTOCOL must use coordinate with SD, can't use POWERCLOCK_ISR
 */




/*
 * callback from module with started event.
 *
 * Pointer to handler func inside a struct whose address is passed to ..._request when starting.
 */
void clockStartedEventHandler(nrf_drv_clock_evt_type_t event) {
	// Called when clock is started.
	// We don't care when the clock starts, we know it will start and thus app_timers beging working.
	//NRFLog::log("LF clock started");
}

nrf_drv_clock_handler_item_s handlers = {
			p_next: nullptr,
			event_handler: clockStartedEventHandler
	};

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
	/*
	 * Ignore return NRF_ERROR_MODULE_ALREADY_INITIALIZED.
	 * It is harmless.
	 * This depends on init() returning only two errors: success or already initialized.
	 */
	(void) nrf_drv_clock_init();
}



void LowFreqClockCoordinated::start() {
	/*
	 * Clock source is configured by ???
	 */

	nrf_drv_clock_lfclk_request(nullptr);
	// When need callback on started
	// nrf_drv_clock_lfclk_request(&handlers);

	_isStarted = true;
	/*
	 * !!! Not wait for any event.
	 * not assert isRunning()
	 * See Nordic docs re callback on started.
	 */
}


bool LowFreqClockCoordinated::isStarted() {
	/*
	 * Lower API does not support, use local flag
	 */
	return _isStarted;
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
	return nrf_drv_clock_lfclk_is_running();
}



void LowFreqClockCoordinated::spinUntilRunning() {
	while( !isRunning()) {}
}
