
#include <clock/longClock.h>
#include <cassert>
#include <inttypes.h>

#include "timer.h"

#include "../drivers/lowFrequencyClock.h"
#include "../drivers/counter.h"


extern "C" { void RTC0_IRQHandler(void); }


/*
 * Private data.
 * If you use the underlying peripherals elsewhere, you must coordinate.
 */
namespace {

// TODO use pure classes
LowFrequencyClock lowFrequencyClock;
Counter counter;



/*
 * MSB of LongClock
 * !!! Volatile: incremented by overflow interrupt handler (a thread), and read by main thread.
 * See below re disable interrupts for critical sections.
 */
volatile uint32_t mostSignificantBits;


/*
 * !!! Does not guarantee oscillator is running.
 */
void startXtalOscillator() {
	lowFrequencyClock.configureXtalSource();
	// assert source is LFXO

	lowFrequencyClock.start();
	// not assert(lowFrequencyClock.isRunning());
}


} // namespace




/*
 * One handler for many interrupt sources (overflow, and compare regs)
 * Many sources can be pending, so handle them all.
 *
 * Each IRQ may callback more than one user of Timer!
 *
 * Overrides weak default handler defined by gcc_startup_nrf52.c.
 */
extern "C" {	// Binding must be "C" to override default handler.

__attribute__ ((interrupt ("RTC_IRQ")))
void
RTC0_IRQHandler(void)
{
	/*
	 * Dispatch on event type: overflow or compare reg match
	 *
	 * !!! More than one event may be set: handle them all
	 * !!! The interrupt can come with no events, since SW can pend the interrupt in the NVIC
	 */

	// TODO call counterISR
	// Source event is overflow
	if ( counter.isOverflowEvent() ) {
		mostSignificantBits++;
		counter.clearOverflowEvent();
		// assert interrupt still enabled
		// assert counter is near zero (it rolled over just before the interrupt)
	}


	Timer::timerISR();



	/*
	 * If any events have triggered after we checked them,
	 * they will still trigger an interrupt and this handler will be called again.
	 */
}
}	// extern "C"




void LongClock::start() {

	resetToNearZero();
	// Later, a user (say SleepSyncAgent) can reset again

	// RTC requires some LFC started, here use LFXO
	startXtalOscillator();
	/*
	 * Oscillator might not be running (startup time.)
	 * Oscillator source might temporarily be LFRC instead of LFXO.
	 */

	// Product anomaly 20 on nRF52 says do this
	counter.stop();

	// Docs don't say this can't be done while counter is running
	counter.configureOverflowInterrupt();
	// assert overflow interrupt is enabled in device
	// assert RTC0_IRQ is enabled (for counter and timers)
	// mostSignificantBits will increment on overflow

	// assert prescaler is default of 0, i.e. 30uSec tick

	counter.start();

	// This could be done elsewhere
	Timer::initTimers();
	// assert compareRegisters are configured by default to disabled interrupt w/ nullptr callbacks


	/*
	 * not assert rc or xtal oscillator isRunning.
	 * Accuracy might be low until isRunning.
	 * The RC oscillator will running first, but even it may not be running.
	 * LFRC starts in 600uSec (nrf52)
	 * LFXO starts in 0.25Sec
	 */
	// assert counter is started.
	// assert interrupt enabled for overflow

	// nrf51 anomaly 72 irrelevant: LFCLK is running, permanently
}


void LongClock::resetToNearZero(){
	mostSignificantBits = 0;
}


LongTime LongClock::nowTime() {

	/*
	 * Implementation: use Lamport's Rule.
	 * To correctly catenate two 32-bit (sic) components of the 64-bit clock.
	 *
	 * The components are volatile and incremented by separate threads.
	 * ISR increments mostSignificantBits (separate thread, at any instant.)
	 * leastSignificantBits are incremented by hw (separate thread.)
	 * This routine is in a third separate thread (main thread.)
	 */
	uint32_t firstMSBRead, secondMSBRead;
	OSTime LSBRead;
	do {
		firstMSBRead = mostSignificantBits;
		LSBRead = counter.ticks();
		secondMSBRead = mostSignificantBits;
	}
	while (firstMSBRead != secondMSBRead);

	/*
	 * Catenate MSB and LSB reads.  Portable?
	 */
	LongTime result = firstMSBRead;
	result = result << OSClockCountBits;	// Left shift result, fill LSB with zero
	/*
	 * Bit-wise OR the 32-bit LSBRead into lower 32-bit of result.  Addition would work also.
	 * Assert !(LSBRead & 0xFF000000) i.e. high order 8-bits are zeroes
	 */
	result = result | LSBRead;
	return result;

	// XXX Monotonicity by design, but add an assertion here
	/*
	 * i.e.
	 * priorResult initially 0
	 * assert(thisResult > priorResult)
	 * priorResult = thisResult
	 */
}


OSTime LongClock::osClockNowTime() {
	return counter.ticks();
}


bool LongClock::isOSClockRunning(){
	return lowFrequencyClock.isRunning();
}
