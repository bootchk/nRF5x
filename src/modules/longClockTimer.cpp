
#include <cassert>
#include <inttypes.h>

#include "../modules/longClockTimer.h"
#include "../drivers/lowFrequencyClock.h"
#include "../drivers/counter.h"
#include "../drivers/compareRegister.h"

extern "C" { void RTC0_IRQHandler(void); }


/*
 * Private data.
 * If you use the underlying peripherals elsewhere, you must coordinate.
 */
namespace {

LowFrequencyClock lowFrequencyClock;
Counter counter;



/*
 * MSB of LongClock
 * !!! Volatile: incremented by overflow interrupt handler (a thread), and read by main thread.
 * See below re disable interrupts for critical sections.
 */
volatile uint32_t mostSignificantBits;

/*
 * RTC device has three compare registers, each generating event/interrupt.
 * All the events are handled by one ISR (RTC0_ISRHandler)
 * This class owns a facade on the counter registers, and a knows a callback for each's interrupt.
 * Callback is also used as a flag for 'started'
 */
TimerCallback timerCallback[3];
CompareRegister compareRegisters[3];


/*
 * !!! Does not guarantee oscillator is running.
 */
void startXtalOscillator() {
	lowFrequencyClock.configureXtalSource();
	// assert source is LFXO

	lowFrequencyClock.start();
	// not assert(lowFrequencyClock.isRunning());
}


// TODO this could be done as initializers, not at runtime and would then be in ROM?
/*
 * Init the facades to the RTCTimers, i.e. the addresses and masks of each compare register.
 *
 * This does not guarantee the state of the hw compare registers
 * (but typically, all are in POR reset state i.e. disabled.)
 */
void initCompareRegs() {
	// This is expanded because the hw constants are defined by unparameterized macros
	// Parameters of compareRegisters are fixed by hw design
	compareRegisters[0].init(
			NRF_RTC_EVENT_COMPARE_0,
			//RTC_EVTEN_COMPARE0_Msk,
			NRF_RTC_INT_COMPARE0_MASK,
			0	// index
	);
	compareRegisters[1].init(
			NRF_RTC_EVENT_COMPARE_1,
			//RTC_EVTEN_COMPARE1_Msk,
			NRF_RTC_INT_COMPARE1_MASK,
			1	// index
	);
	compareRegisters[2].init(
			NRF_RTC_EVENT_COMPARE_2,
			//RTC_EVTEN_COMPARE2_Msk,
			NRF_RTC_INT_COMPARE2_MASK,
			2	// index
	);

	timerCallback[0] = nullptr;
	timerCallback[1] = nullptr;
	timerCallback[2] = nullptr;

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
	 * !!! More than one event may be pending: handle them all
	 */

	// Source event is overflow
	if ( counter.isOverflowEvent() ) {
		mostSignificantBits++;
		counter.clearOverflowEvent();
		// assert interrupt still enabled
		// assert counter is near zero (it rolled over just before the interrupt)
	}

	// Source event is compare register match event
	// Handle all compare regs: 0-1
	if ( compareRegisters[First].isEvent() ) {
		// Sleep timer expired
		timerCallback[First](SleepTimerCompare);	// call callback
		LongClockTimer::cancelTimer(First);
	}
	else {
		/*
		 * First Timer is unique: used for a sleep loop.
		 * If it is active, it will have woken by whatever event this is.
		 * Pass the callback the reason for wake, so it can sleep again.
		 */
		if ( LongClockTimer::isTimerStarted(First) ) {
			timerCallback[First](OverflowOrOtherTimerCompare);
		}
	}

	if ( compareRegisters[Second].isEvent() ) {
			timerCallback[Second](OverflowOrOtherTimerCompare);	// call callback
			LongClockTimer::cancelTimer(Second);
		}
	// User of second timer doesn't sleep on it.


	/*
	 * If any events have triggered after we checked them,
	 * they will still trigger an interrupt and this handler will be called again.
	 */
}
}	// extern "C"


void LongClockTimer::start(Nvic* nvic) {

	resetToNearZero();
	// Later, a user (say SleepSyncAgent) can reset again

	// RTC requires LFC started
	startXtalOscillator();
	/*
	 * Oscillator might not be running (startup time.)
	 * Oscillator source might temporarily be RC instead of XTAL.
	 */

	// Product anomaly 20 on nRF52 says do this
	counter.stop();

	// Docs don't say this can't be done while counter is running
	counter.configureOverflowInterrupt();
	// assert overflow interrupt is enabled in device
	// assert RTC0_IRQ is enabled (for counter and timers)
	// mostSignificantBits will increment on overflow

	// assert prescaler is default of 0, i.e. 30uSec tick

	// Counter knows nvic
	counter.init(nvic);
	counter.start();

	initCompareRegs();

	/*
	 * not assert rc or xtal oscillator isRunning.
	 * Accuracy might be low until isRunning.
	 * The RC oscillator will running first, but even it may not be running.
	 * LFRC starts in 600uSec (nrf52)
	 * LFXO starts in 0.25Sec
	 */
	// assert counter is started.
	// assert interrupt enabled for overflow
	// assert compareRegisters are configured by default to disabled interrupt w/ nullptr callbacks

	// nrf51 anomaly 72 irrelevant: LFCLK is running, permanently
}


void LongClockTimer::resetToNearZero(){
	mostSignificantBits = 0;
}


LongTime LongClockTimer::nowTime() {

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


/*
 * This must be kept short.
 * The timeout could be as little as 3 ticks, or 3*30 = 90uSec, which allows about 1440 instructions.
 */
void LongClockTimer::startTimer(
		TimerIndex index,
		OSTime timeout,
		TimerCallback aTimeoutCallback){
	/*
	 * These assertions do not need be short to prevent sleep forever.
	 * But these assertions do affect the accuracy of the timeout
	 * (the more time we spend here, the later the real timeout will occur
	 * after the time for which the timeout was calculated.
	 */
	assert(timeout < MaxTimeout);
	assert(timeout >= MinTimeout);
	assert(index < CountTimerInstances);
	// assert RTC0_IRQ enabled (enabled earlier for Counter, and stays enabled.

	// Not legal to start Timer already started and not timed out or canceled.
	if (isTimerStarted(index)) {
		assert(false);
		return;	// No error result, must be tested with assertions enabled.
	}

	// remember callback
	timerCallback[index] = aTimeoutCallback;

	/*
	 * Setting timeout and enabling interrupt must be close together,
	 * else counter exceeds compare already, and no interrupt till much later after counter rolls over.
	 */
	compareRegisters[index].set(timeout);
	// Compare match must not have happened yet, else no interrupt.
	// Since interrupt from compare not enabled yet.
	compareRegisters[index].enableInterrupt();
	/*
	 * The interval between setting the compare register and returning must be kept short,
	 * since the caller's continuation is usually: sleep until interrupt.
	 * The interrupt must not occur before the caller sleeps.
	 * A clock overflow or Timer[Second] interrupt and ISR could occur in that interval.
	 * The timeout could be as little as 3 ticks, or 3*30 = 90uSec, which allows about 1440 instructions.
	 */
}

bool LongClockTimer::isTimerStarted(TimerIndex index) {
	return ! (timerCallback[index] == nullptr);
}

void LongClockTimer::cancelTimer(TimerIndex index){
	/*
	 * Legal to cancel Timer that has not been started.
	 * Legal to call from IRQ or main thread.
	 *
	 * Possible race: Timer IRQ may expire in the middle of this,
	 * so whatever flag the Timer sets may be set even after this is called.
	 *
	 * We clear compare reg event, so it would not be set in a race.
	 */
	compareRegisters[index].disableInterrupt();
	timerCallback[index] = nullptr;
	compareRegisters[index].clearEvent();
	/*
	 * One-shot: assert:
	 * - compare interrupt is disabled.
	 * - compare event is cleared
	 * - compare event is disabled
	 * - callback is cleared
	 * Counter continues and compare reg still set, but it can't fire.
	 */
}


bool LongClockTimer::isOSClockRunning(){
	return lowFrequencyClock.isRunning();
}
