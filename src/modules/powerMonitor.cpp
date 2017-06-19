#include "powerMonitor.h"
#include "../drivers/powerComparator.h"


/*
 * Much of the complexity of implementation is:
 * multiplexing simple Vcc measurement with brownout detection.
 * Might be easier to use ADC for simple Vcc measurement.
 */

namespace {

/*
 * No instance of PowerComparator. All class methods
 */

bool _brownoutDetectionMode = false;

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
	// not require not detecting already, this may be superfluous
	if (_brownoutDetectionMode) {

		// set lowest possible threshold the POFCON supports on family
		PowerComparator::setThresholdAndDisable(PowerMonitor::BrownoutThreshold);

		// assert isEventClear()
		PowerComparator::enableInterrupt();
		PowerComparator::enable();
	}
	// assert _brownoutDetectionMode=>PowerComparator is actively detecting brownout and will interrupt
}



/*
 * Agnostic of brownout detection.
 * Requires PowerComparator to be not in use for brownout detection.
 */
bool isVddGreaterThanThreshold(nrf_power_pof_thr_t threshold) {
	bool result;

	// get a new measurement on different threshold
	PowerComparator::setThresholdAndDisable(threshold);
	result = testVddGreaterThanThresholdThenDisable();

	return result;
}

/*
 * This takes care to maintain the alternate use of POFCON: brownout detect.
 *
 * It temporarily disables brownout interrupt, then restores it.
 */
bool isVddGreaterThanThresholdWithBrownoutDetection(nrf_power_pof_thr_t threshold) {
	bool result;

	// disable brownout detection temporarily while we use device for other purpose
	PowerComparator::disableInterrupt();

	// get result to be returned
	result = isVddGreaterThanThreshold(threshold);

	/*
	 * After every call, restore PowerComparator to brownout detect if Vcc > brownout threshold.
	 * We don't want an interrupt immediately if Vcc < brownout threshold.
	 */
	if (isVddGreaterThanThreshold(PowerMonitor::BrownoutThreshold)) {
		tryEnableBrownoutDetection();
	}

	/*
	 * Assert result valid.
	 * Assert brownout detection enabled if enableBrownoutDetection() was called previously
	 * else POFCON disabled and threshold is indeterminate.
	 */
	return result;
}


}  // namespace



void PowerMonitor::enableBrownoutDetectMode() {
	_brownoutDetectionMode = true;
	// Detection not effected until first call to isVddGreaterThan2xxx
}


void PowerMonitor::disableBrownoutDetection() {
	PowerComparator::disableInterrupt();
	PowerComparator::disable();
	_brownoutDetectionMode = true;
}

/*
 * !!! Side effect: enable brownout detection.
 */
bool PowerMonitor::isVddGreaterThan2_1V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V21); }
bool PowerMonitor::isVddGreaterThan2_3V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V23); }
bool PowerMonitor::isVddGreaterThan2_5V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V25); }
bool PowerMonitor::isVddGreaterThan2_7V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V27); }
#ifdef NRF52
bool PowerMonitor::isVddGreaterThan2_8V() { return isVddGreaterThanThresholdWithBrownoutDetection(NRF_POWER_POFTHR_V28); }
#endif


