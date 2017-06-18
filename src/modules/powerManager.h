
#pragma once

/*
 * Four levels gives five ranges.
 */
enum class VoltageRange { BelowUltraLow, UltraLowToLow, LowToMedium, MediumToHigh, HighToExcess, AboveExcess};

/*
 * Understands:
 * - power levels and ranges
 *
 * excessVoltage:
 * The power supply may be unregulated (solar)
 * and capable of system damaging voltages (e.g. 4.8V exceeding mcu Vmax of 3.6V.)
 *
 * Depends on some device that read system Vcc (Vdd).
 * Voltage levels on some charge storage (capacitor) indicates power level.
 *
 * Brownout is:
 * - real
 * - detected
 * Detected brownout only means that the mcu will soon suffer real brownout (a reset, BOR.)
 * Typically, the handler for detected brownout quits the app anyway.
 */
class PowerManager {

public:
	static void init();

	static void enableBrownoutDetectMode();

	/*
	 * Levels
	 */
	static bool isPowerExcess();	// Above Vmax of chip 3.6V
	static bool isPowerAboveUltraHigh();
	static bool isPowerAboveHigh();
	static bool isPowerAboveMedium();
	static bool isPowerAboveLow();
	static bool isPowerAboveUltraLow();

	/*
	 * Ranges
	 */
	static VoltageRange getVoltageRange();
};
