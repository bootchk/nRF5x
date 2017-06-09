

#pragma once

/*
 * Default power supply is LDO.
 * Another more efficient (at certain high voltages) power supply is DCDC.
 * See Nordic docs: this saves power for certain high Vcc and large currents (e.g. radio)
 *
 * Enabling DCDC converter lets the radio control it automatically.  Enabling does not turn it on.
 *
 * DCDC converter requires Vcc >= 2.1V and should be disabled below that?
 *
 * There is a startup time before transmission can subsequently be started.
 */
class DCDCPowerSupply{
public:
	static void enable();
	static void disable();
};
