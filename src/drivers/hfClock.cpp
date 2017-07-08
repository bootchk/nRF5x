#include <cassert>

//#include <nrf.h>	// direct access to NRF_CLOCK
#include <nrf_clock.h>	// HAL

#include "hfClock.h"
#include "nvic.h"

#include "../clock/sleeper.h"




void HfCrystalClock::enableInterruptOnRunning() {

	// Requires this, but not implemented
	// assert(Nvic::isEnabledPowerClockIRQ());

	/*
	 * EVENT_HFCLKSTARTED signals HFXO clock is running stably !!!!
	 * The name seems wrong:
	 *  - it doesn't specify the XO
	 *  - it says "STARTED" not "RUNNING"
	 *  But see the docs.
	 */
	nrf_clock_int_enable(NRF_CLOCK_INT_HF_STARTED_MASK);
}

bool HfCrystalClock::isInterruptEnabledForRunning() {
	return nrf_clock_int_enable_check(NRF_CLOCK_INT_HF_STARTED_MASK);
}

#ifdef NOTUSED
void disableInterruptOnRunning() {
	nvic->disablePowerClockIRQ();
	nrf_clock_int_disable(NRF_CLOCK_INT_HF_STARTED_MASK);
}
#endif





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


void HfCrystalClock::init(){

	enableInterruptOnRunning();
	// Leave it enabled
}



/*
 * Only trigger start task.
 * Does not ensure isRunning
 * Does not ensure interrupts enabled
 */
void HfCrystalClock::start(){

	// Enable the High Frequency clock to the system as a whole
	nrf_clock_event_clear(NRF_CLOCK_EVENT_HFCLKSTARTED);
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTART);
	// assume HAL is correct and write cache flushed

#ifdef OLD
	non-HAL
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
	// No need to flush ARM write cache, the read below will do it
	NRF_CLOCK->TASKS_HFCLKSTART = 1;
#endif
}




bool HfCrystalClock::isRunning(){
	/*
	 * This doesn't use Hal
	 * Also it might be wrong: docs are not clear that event guarantees running
	 * (but I think it is just misnamed, and HFCLKSTARTED really means running.)
	 * while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) { }
	 */
	return nrf_clock_hf_is_running( NRF_CLOCK_HFCLK_HIGH_ACCURACY );
}


void HfCrystalClock::startAndSleepUntilRunning() {
	/*
	 * Illegal to call if already running.
	 * In that case, there might not be an event or interrupt to wake,
	 * or the interrupt could occur quickly after we start() but before we sleep (WFI)
	 */
	assert( !isRunning() );
	assert(!nrf_clock_event_check(NRF_CLOCK_EVENT_HFCLKSTARTED));

	// Interrupt must be enabled because we sleep until interrupt
	assert (isInterruptEnabledForRunning());

	/*
	 * We should not be sleeping, but other low-priority reasons such as BrownoutWarning etc.
	 * could be set and lost by this clear.
	 */
	Sleeper::clearReasonForWake();
	start();
	/*
	 * Event may have come already, but there is not a race to clear reason.
	 * sleepUntilEvent does NOT clearReasonForWake.
	 */

	// Blocking
	Sleeper::sleepUntilEvent(ReasonForWake::HFClockStarted);

	// assert ISR cleared event.

	// Do not disable interrupt.

	assert(isRunning());
}


#ifdef NOTUSED
void HfCrystalClock::startAndWaitUntilRunning(){

	// Enable the High Frequency clock to the system as a whole
	start();

	// Spin (wasting energy)
	while ( !isRunning()  ) {}

	// FUTURE use nrf_driver/hal functions
	assert(NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKSTAT_SRC_Msk);	// 1 == Xtal
	assert(NRF_CLOCK->HFCLKSTAT & CLOCK_HFCLKRUN_STATUS_Msk);	// 1 == running
}
#endif


void HfCrystalClock::stop(){
	nrf_clock_task_trigger(NRF_CLOCK_TASK_HFCLKSTOP);
	// non-HAL NRF_CLOCK->TASKS_HFCLKSTOP = 1;

	/*
	 *  Will generate event, but interrupt not enabled, and we don't wait for event indicating stopped
	 */
	// ?? Do we need a delay here?
	assert(!isRunning());
}

