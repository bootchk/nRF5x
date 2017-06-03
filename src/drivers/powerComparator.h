
/*
 * Driver for nrf5x family PowerFailureComparator (POFCON) device.
 *
 * This is portable across nrf51 and nrf52 families.
 * (Unlike the SAADC.)
 *
 * nrf52 supports other discrete values e.g. 2_0.
 * Not implemented here since not portable.
 *
 * Functions return value which is discrete (bool), not continuous (float).
 * If you need a float type measurement, use SAADC.
 *
 * Not to be confused with any other comparator:
 * - COMP
 * - LPCOMP
 *
 * This driver is probably not compatible with other uses of the PowerFailComparator device:
 * to interrupt when power is failing to give the OS time to gracefully shut down.
 * There you are interested  being asynchronously notified of falling edges.
 * Here we are interested in synchronously the condition: above a given level.
 */

#include <nrf_power.h>	// hal

class PowerComparator {
public:
	static void enable();
	static void disable();
	static bool isDisabled();
	static void disableInterrupt();

	/*
	 * Did device generate event for the configured threshold?
	 * Does not clear the event.
	 */
	static bool isPOFEvent();
	static void clearPOFEvent();

	static void setThresholdAndDisable(nrf_power_pof_thr_t threshold); 	// Use NRFSDK hal enum

	static void delayForPOFEvent();
};
