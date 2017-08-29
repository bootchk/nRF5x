
#include <cassert>
#include <nrf.h>
#include <nrf_power.h>

#include "mcu.h"


#include "powerComparator.h"

// Used by ISR
// Call layer above, radioSoC
// TODO for more independence these should be callbacks registered from above
#include <services/brownoutRecorder.h>
#include <clock/sleeper.h>
#include <modules/powerMonitor.h>




namespace {

static void setThresholdMaskAndDisable(nrf_power_pof_thr_t thresholdMask) {
	/*
	 * Set and clear multiple bits:
	 * - several threshold bits to a bit pattern
	 * - the enable/disable bit to zero.
	 */
	// Shift mask into position in register
	NRF_POWER->POFCON = thresholdMask << POWER_POFCON_THRESHOLD_Pos;
	MCU::flushWriteCache();
	assert(PowerComparator::isDisabled());
}


}  // namespace



// TODO use hal, since it is family nrf52 independent

/*
 * Implementation notes:
 * Uses the power fail comparator, but using just an even, not an interrupt.
 *
 * Does not use the SDK HAL since it smells unpolished.
 *
 * Reads to flush ARM write cache must not be optimized out,
 * the write that must be flushed is a trigger to an external device
 * that must occur timely, i.e. when the write is made
 * and not at some unknown time in the future when memory is read.
 *
 * These thresholds are common to NRF51 and NRF52.
 * NRF52 family has finer grained thresholds (more thresholds), but highest is only 2.8V
 */

/*
 * Testing procedure for brownout detection/writing to flash:
 * Use a NRF52DK with custom board connected by SW and having separate variable power supply.
 * Set power supply to 3V.
 * Start debugger and execute a main that harnesses this code.
 * Lower the power supply to below 2.3V (or whatever you have set for brownout detection.)
 * The debugger should stop at the BKPT below.
 * Word 19 of UICR should contain the PC where the fault occurred.
 * Use Eclipse memory view to view UICR at 0x10001080, use unsigned int rendering to view fault address in decimal.
 */

void PowerComparator::powerISR() {
	if (nrf_power_event_check(NRF_POWER_EVENT_POFWARN)) {

		/*
		 * Brownout: write PC to flash so we can analyze later where in the app we brownout.
		 */
		BrownoutRecorder::recordToFlash();
#ifdef NOT_USED
		BrownoutRecorder::recordToFlash(faultAddress);
#endif

		/*
		 * Typically little further execution is possible (power is failing).
		 *
		 * Alternatives:
		 * 1) stop execution (at time of fault.)
		 * 2) continue, signal to others BrownoutWarning, and wait for actual BOR
		 *
		 * 1) BKPT causes an additional hard fault on Cortext M0
		 * __asm("BKPT #0\n") ; // Break into the debugger, if it is running
		 * 1) resetOrHalt();
		 *   will enter infinite loop
		 */

		/*
		 *  2) Proceed and wait for actual BOR
		 */
		// Signal
		// TODO prioritize
		Sleeper::setReasonForWake(ReasonForWake::BrownoutWarning);

		/*
		 * Disable further POFWARN events, until enabled later.
		 * The app may enable them again later.
		 * Only here do we understand how to clear EVENT.
		 * PowerMonitor only understands disabling.
		 */
		nrf_power_event_clear(NRF_POWER_EVENT_POFWARN);
		PowerMonitor::disableBrownoutDetection();
	}
}




void __attribute__((optimize("O0")))
PowerComparator::enable() {
	/*
	 * BIS bitset the enable bit.
	 * This triggers event if Vcc less than threshold.
	 */
	NRF_POWER->POFCON |= POWER_POFCON_POF_Msk;
	MCU::flushWriteCache();

	assert( NRF_POWER->POFCON & POWER_POFCON_POF_Msk);	// ensure enable bit was set
	// hw ensurs event is set if power is less than threshold
}



void __attribute__((optimize("O0")))
PowerComparator::disable() {
	// BIC bitclear the enable bit
	NRF_POWER->POFCON &= ~ POWER_POFCON_POF_Msk;
	MCU::flushWriteCache();

	assert( ! (NRF_POWER->POFCON & POWER_POFCON_POF_Msk) );
}


bool PowerComparator::isDisabled() {
	// POFCON POF bit equal to zero => disabled
	return (NRF_POWER->POFCON & POWER_POFCON_POF_Msk) == 0;
}


bool PowerComparator::isPOFEvent() { return NRF_POWER->EVENTS_POFWARN == 1; }


void __attribute__((optimize("O0")))
PowerComparator::clearPOFEvent() {
	// Require disabled, else may be immediately reset after clearing
	assert(isDisabled());

	NRF_POWER->EVENTS_POFWARN = 0;
	MCU::flushWriteCache();

	assert( not isPOFEvent());	// ensure
}

/*
 * !!! Implementation is correct even if other interrupts from same source are used.
 * In INTCLR, only a 1-bit clears an interrupt source, a 0-bit has no effect on interrupts.
 *
 * Some family members have interrupts for EVENTS_SLEEPENTER, etc.
 *
 * !!! Note this interrupt is handled by POWER_CLOCK_IRQHandler, shared with Clock device.
 */
void PowerComparator::disableInterrupt() {
	NRF_POWER->INTENCLR = 1 << POWER_INTENCLR_POFWARN_Pos;
}

void PowerComparator::enableInterrupt() {
	NRF_POWER->INTENSET = 1 << POWER_INTENCLR_POFWARN_Pos;
}




/*
 * Set threshold and disable bits.
 * Device will not compare until enabled.
 *
 * Alternative is to set threshold without clearing disable bit, using mask POWER_POFCON_POF_Msk
 */
void PowerComparator::setThresholdAndDisable(PowerThreshold threshold) {
	/*
	 * For API independence, convert from enum to platform specific bitmask
	 */
	nrf_power_pof_thr_t thresholdMask;
	switch(threshold){
	case PowerThreshold::V2_1:
		thresholdMask = NRF_POWER_POFTHR_V21;
		break;
	case PowerThreshold::V2_3:
		thresholdMask = NRF_POWER_POFTHR_V23;
		break;
	case PowerThreshold::V2_5:
		thresholdMask = NRF_POWER_POFTHR_V25;
		break;
	case PowerThreshold::V2_7:
		thresholdMask = NRF_POWER_POFTHR_V27;
		break;
	// TODO ifdef NRF52
	case PowerThreshold::V2_8:
		thresholdMask = NRF_POWER_POFTHR_V27;
		break;
	}

	setThresholdMaskAndDisable(thresholdMask);
}


// Platform specific
#ifdef NRF52
	/*
	 * Alternative: V17, but that might not leave enough power to record brownout to flash?
	 */
	static const nrf_power_pof_thr_t BrownoutThreshold = NRF_POWER_POFTHR_V21;
#else
	static const nrf_power_pof_thr_t BrownoutThreshold = NRF_POWER_POFTHR_V21;
#endif


void PowerComparator::setBrownoutThresholdAndDisable() {
	setThresholdMaskAndDisable(BrownoutThreshold);
}

/*
 * Delay from POFCON enable until event is generated.
 * Testing shows this is necessary,
 * Nordic docs don't document what delay is necessary.
 *
 *
 * Here, for the NRF52, delay for two cycles of the peripheral bus (at 1/4 the rate of the cpu bus) i.e. 8 cpu cycles.
 * This ensures that the POFCON sees the write in the first peripheral bus cycle,
 * and then can generate the event in the next peripheral bus cycle.
 * I am not sure about any of this.
 *
 * This must not be optimized out.
 */
void PowerComparator::delayForPOFEvent() {
#ifdef NRF52
	asm ("nop");
	asm ("nop");
	asm ("nop");
	asm ("nop");

	asm ("nop");
	asm ("nop");
#endif
	// NRF51 peripheral bus freq matches cpu freq so a few cyles is enough
	asm ("nop");
	asm ("nop");

}
