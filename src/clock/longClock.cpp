

#include <cassert>
#include <inttypes.h>

#include <clock/longClock.h>

#include "timer.h"
#include "../drivers/lowFrequencyClock.h"
#include "../drivers/counter.h"



/*
 * Private data.
 * If you use the underlying peripherals elsewhere, you must coordinate.
 */
namespace {

// TODO use pure classes
LowFrequencyClock lowFrequencyClock;

LongTime priorNow = 0;	// for assertion


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



void LongClock::longClockISR() {
	if ( Counter::isOverflowEvent() ) {
		mostSignificantBits++;
		Counter::clearOverflowEventAndWaitUntilClear();
		// assert interrupt still enabled
		// assert counter is near zero (it rolled over just before the interrupt)
		// assert event was definitely cleared (more than 4 clock cycles ago on Cortex M4.)
	}
}

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
	Counter::stop();

	// Docs don't say this can't be done while counter is running
	Counter::configureOverflowInterrupt();
	// assert overflow interrupt is enabled in device
	// assert RTC0_IRQ is enabled (for counter and timers)
	// mostSignificantBits will increment on overflow

	// assert prescaler is default of 0, i.e. 30uSec tick

	Counter::start();

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
		// Since mostSignficantBits is volatile, optimization does not optimize away the "consecutive" reads.
		firstMSBRead = mostSignificantBits;
		LSBRead = Counter::ticks();
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

	/*
	 * Monotonicity by design, but assert here
	 */
	assert(result >= priorNow);
	priorNow = result;

	return result;
}


OSTime LongClock::osClockNowTime() {
	return Counter::ticks();
}


bool LongClock::isOSClockRunning(){
	return lowFrequencyClock.isRunning();
}
