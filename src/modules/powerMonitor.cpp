#include "powerMonitor.h"
#include "../drivers/powerComparator.h"




namespace {

/*
 * No instance of PowerComparator. All class methods
 */

bool _brownoutDetectionState = false;

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


/*
 * Enable brownoutDetection if enableBrownoutDetection() was called previously.
 */
void tryEnableBrownoutDetection() {
	if (_brownoutDetectionState) {

		// set lowest possible threshold the POFCON supports on family
#ifdef NRF52
		PowerComparator::setThresholdAndDisable(NRF_POWER_POFTHR_V17);
#else
		PowerComparator::setThresholdAndDisable(NRF_POWER_POFTHR_V21);
#endif

		// assert isEventClear()
		PowerComparator::enableInterrupt();
		PowerComparator::enable();
	}
}


/*
 * This takes care to maintain the alternate use of POFCON: brownout detect.
 *
 * It temporarily disables brownout interrupt, then restores it.
 */
bool isVddGreaterThanThreshold(nrf_power_pof_thr_t threshold) {
	bool result;

	// disable brownout detection temporarily while we use device for other purpose
	PowerComparator::disableInterrupt();

	// get a new measurement on different threshold
	PowerComparator::setThresholdAndDisable(threshold);
	result = testVddGreaterThanThresholdThenDisable();

	tryEnableBrownoutDetection();

	/*
	 * Assert result valid.
	 * Assert brownout detection enabled if enableBrownoutDetection() was called previously
	 * else POFCON disabled and threshold is indeterminate.
	 */
	return result;
}



}  // namespace



void PowerMonitor::enableBrownoutDetection() { _brownoutDetectionState = true; }


bool PowerMonitor::isVddGreaterThan2_1V() { return isVddGreaterThanThreshold(NRF_POWER_POFTHR_V21); }
bool PowerMonitor::isVddGreaterThan2_3V() { return isVddGreaterThanThreshold(NRF_POWER_POFTHR_V23); }
bool PowerMonitor::isVddGreaterThan2_5V() { return isVddGreaterThanThreshold(NRF_POWER_POFTHR_V25); }
bool PowerMonitor::isVddGreaterThan2_7V() { return isVddGreaterThanThreshold(NRF_POWER_POFTHR_V27); }


