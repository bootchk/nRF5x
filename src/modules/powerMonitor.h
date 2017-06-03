
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
 *
 * !!! Brownout detection starts after the first call to isVddGreaterThan2_xV
 * Can not disable brownout detection.
 */

class PowerMonitor {
public:
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
