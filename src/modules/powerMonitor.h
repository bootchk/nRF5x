#pragma once

#include <nrf_power.h>	// hal, POF threshold constants


/*
 * Adaptor to PowerComparator.
 *
 * This knows how to use PowerComparator for two purposes:
 * - measuring power
 * - monitoring for brownout
 *
 * The two purposes require coordination:
 * monitoring must be turned off while measuring power.
 *
 * !!! Brownout detection starts with first call to enableBrownoutDetection
 * Can not disable brownout detection.
 */

class PowerMonitor {

public:

	/*
	 * NRF5x family constrains lowest possible threshold:
	 * - nrf51: NRF_POWER_POFTHR_V21
	 * - nrf52: NRF_POWER_POFTHR_V17
	 * But you can change this to higher value.
	 */
#ifdef NRF52
		static const nrf_power_pof_thr_t BrownoutThreshold = NRF_POWER_POFTHR_V17;
#else
		static const nrf_power_pof_thr_t BrownoutThreshold = NRF_POWER_POFTHR_V23;
#endif


	/*
	 * Sets mode.
	 * Brownout detection starts after first call so isVddGreater...
	 *
	 * Enables at lowest threshold that POFCON can detect.
	 * Which may be lower/higher than you want.
	 */
	static void enableBrownoutDetectMode();


	/*
	 * Simple tests of Vdd thresholds.
	 *
	 * These have side effects on the PowerFailureComparator device resource:
	 * they leave it disabled.
	 */
	// These are portable nrf51 and nrf52
	static bool isVddGreaterThan2_1V();
	static bool isVddGreaterThan2_3V();
	static bool isVddGreaterThan2_5V();
	static bool isVddGreaterThan2_7V();
};
