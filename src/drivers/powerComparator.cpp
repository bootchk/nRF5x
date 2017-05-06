
#include <cassert>
#include "nrf.h"
#include "mcu.h"

#include "powerComparator.h"

/*
 * Implementation notes:
 * Uses the power fail comparator, but using just an even, not an interrupt.
 *
 * Reads to flush ARM write cache must not be optimized out,
 * the write that must be flushed is a trigger to an external device
 * that must occur timely, i.e. when the write is made
 * and not at some unknown time in the future when memory is read.
 *
 * These thresholds are common to NRF51 and NRF52.
 * NRF52 family has finer grained thresholds (more thresholds), but highest is only 2.8V
 */

namespace {

/*
 * Set threshold and disable bits.
 * Device will not compare until enabled.
 *
 * Alternative is to set threshold without clearing disable bit, using mask POWER_POFCON_POF_Msk
 */
void setThresholdAndDisable(uint32_t threshold) {
	/*
	 * Set and clear multiple bits:
	 * - several threshold bits to a bit pattern
	 * - the enable/disable bit to zero.
	 */
	NRF_POWER->POFCON = threshold << POWER_POFCON_THRESHOLD_Pos;
	MCU::flushWriteCache();
	assert(PowerComparator::isDisabled());
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
void delayForPOFEvent() {
	asm ("nop");
	asm ("nop");
	asm ("nop");
	asm ("nop");

	asm ("nop");
	asm ("nop");
	asm ("nop");
	asm ("nop");
}

/*
 * Return result of compare Vdd to threshold.
 * Leaves:
 * - device disabled
 * - event cleared
 *
 */
bool testVddGreaterThanThresholdThenDisable() {
	bool result;

	// Require threshold to be set
	// But some thresholds (V21) have value zero on nrf51

	// clear event so enabling might reset it
	PowerComparator::clearPOFEvent();

	PowerComparator::enable();

	/*
	 * Testing shows that POFCON does not generate event immediately,
	 * at least on nrf52.  Requires a very short delay.
	 */
	delayForPOFEvent();

	/*
	 * Event indicates Vdd less than threshold.
	 * Invert: return true if Vdd greater than threshold.
	 */
	result = ! PowerComparator::isPOFEvent();
	PowerComparator::disable();
	PowerComparator::clearPOFEvent();
	return result;
}

}  // namespace





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
 */
void PowerComparator::disableInterrupt() {
	NRF_POWER->INTENCLR = 1 << POWER_INTENCLR_POFWARN_Pos;
}

bool PowerComparator::isVddGreaterThan2_1V() {

	// FUTURE Ensure our bit twiddling not generate interrupts
	// disableInterrupt();
	// Assume default state of interrupt not enabled.
	// If caller has enabled interrupts, caller must handle them.

	setThresholdAndDisable(POWER_POFCON_THRESHOLD_V21);
	return testVddGreaterThanThresholdThenDisable();
}

bool PowerComparator::isVddGreaterThan2_3V(){
	setThresholdAndDisable(POWER_POFCON_THRESHOLD_V23);
	return testVddGreaterThanThresholdThenDisable();
}
bool PowerComparator::isVddGreaterThan2_5V(){
	setThresholdAndDisable(POWER_POFCON_THRESHOLD_V25);
	return testVddGreaterThanThresholdThenDisable();
}
bool PowerComparator::isVddGreaterThan2_7V(){
	setThresholdAndDisable(POWER_POFCON_THRESHOLD_V27);
	return testVddGreaterThanThresholdThenDisable();
}

