#include "powerMonitor.h"
#include "../drivers/powerComparator.h"

#include <inttypes.h>



// TODO instance or all class methods
namespace {

/*
 * Owns single instance.
 */
PowerComparator powerComparator;



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
	PowerComparator::delayForPOFEvent();

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




bool PowerMonitor::isVddGreaterThan2_1V() {

	// FUTURE Ensure our bit twiddling not generate interrupts
	// disableInterrupt();
	// Assume default state of interrupt not enabled.
	// If caller has enabled interrupts, caller must handle them.

	powerComparator.setThresholdAndDisable(NRF_POWER_POFTHR_V21);
	return testVddGreaterThanThresholdThenDisable();
}

bool PowerMonitor::isVddGreaterThan2_3V(){
	powerComparator.setThresholdAndDisable(NRF_POWER_POFTHR_V23);
	return testVddGreaterThanThresholdThenDisable();
}
bool PowerMonitor::isVddGreaterThan2_5V(){
	powerComparator.setThresholdAndDisable(NRF_POWER_POFTHR_V25);
	return testVddGreaterThanThresholdThenDisable();
}
bool PowerMonitor::isVddGreaterThan2_7V(){
	powerComparator.setThresholdAndDisable(NRF_POWER_POFTHR_V27);
	return testVddGreaterThanThresholdThenDisable();
}

