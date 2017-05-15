
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

typedef void (*funcPtr)();

LowFrequencyClock lowFrequencyClock;
Counter counter;



// MSB of LongClock incremented by overflow interrupt handler
uint32_t mostSignificantBits;

/*
 * RTC device has three compare registers.
 * This class owns them also, and a callback for each's interrupt.
 * Callback is also used as a flag for 'started'
 */
funcPtr timerCallback[3];
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
	}

	// Source event is comparison
	// Loop over compare regs
	for (unsigned int i=0; i<LongClockTimer::CountTimerInstances; i++ ) {
		if ( compareRegisters[i].isEvent() ) {
			timerCallback[i]();	// call callback
			LongClockTimer::cancelTimer((TimerIndex) i);
		}
	}

	/*
	 * If any events have triggered after we checked them,
	 * they will still trigger an interrupt and this handler will be called again.
	 */
}
}	// extern "C"


void LongClockTimer::init(Nvic* nvic) {

	reset();

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


void LongClockTimer::reset(){
	mostSignificantBits = 0;
}


LongTime LongClockTimer::nowTime() {
	OSTime currentOSClockTicks = counter.ticks();
	// Concatenate MSB and LSB.  Portable?
	LongTime result = mostSignificantBits;
	result = result << OSClockCountBits;	// Left shift result, fill LSB with zero
	result = result | currentOSClockTicks;	// Bit-wise OR into LSB.  Addition would work also.
	return result;
}



void LongClockTimer::startTimer(
		TimerIndex index,
		OSTime timeout,
		void (*aTimeoutCallback)()){
	assert(timeout < MaxTimeout);
	assert(timeout >= MinTimeout);
	assert(index < CountTimerInstances);

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
	// interrupt from compare not enabled yet
	compareRegisters[index].enableInterrupt();

	// assert RTC0 IRQ enabled (earlier for Counter.)
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
