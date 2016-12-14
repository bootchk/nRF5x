

#pragma once

/*
 * Default power supply is LDO.
 * Another more efficient (at certain high voltages) power supply is DCDC.
 *
 * Enabling DCDC converter lets the radio control it automatically.  Enabling does not turn it on.
 *
 * DCDC converter requires Vcc >= 2.1V and should be disabled below that?
 */
class PowerSupply {
public:
	static void enableDCDCPower();
};
