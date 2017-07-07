#include <cassert>

#include "clockFacilitator.h"

#include "../drivers/lowFrequencyClock.h"
#include "sleeper.h"


/*
 * Implementation notes:
 * We enable interrupt on LFCLOCKSTARTED, and leave it enabled.
 * The event comes and we disable interrupt but leave the event as a flag to show it is started.
 */
// TODO disable in POWER_CLOCK IRQ

void ClockFacilitator::startLongClockWithSleepUntilRunning(){

	/*
	 * In reverse order of dependencies.
	 */

	LowFrequencyClock::enableInterruptOnStarted();
	LowFrequencyClock::configureXtalSource();
	// assert source is LFXO

	// We start LFXO.  LFRC starts anyway, first, but doesn't generate LFCLOCKSTARTED?
	LowFrequencyClock::start();

	// Race: must sleep before LFCLOCKSTARTED event comes.  Takes .25 seconds for LFXO.  Takes .6mSec for LFRC.

	Sleeper::sleepUntilEvent(ReasonForWake::LFClockStarted);

	assert(LowFrequencyClock::isRunning());
}
