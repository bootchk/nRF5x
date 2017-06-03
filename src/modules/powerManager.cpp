
#include "powerManager.h"
#include "powerMonitor.h"
#include "../drivers/adc.h"

namespace {

/*
 * Uses PowerMonitor and ADC, somewhat arbitrarily.
 *
 * You could use only the ADC (on nrf51) or SAADC (on nrf52), could define whatever levels you wish.
 *
 * On the nrf52 you could use only the PowerMonitor which defines more levels, possible as high as 3.6V needed for isPowerExcess.
 */
PowerMonitor powerMonitor;

#ifdef NRF51
ADC adc;
#endif
}  // namespace



/*
 * Specific to my application:
 *
 * Given:
 *  - storage capacitor is certain joules.
 *  - power supply delivers about 10uA @2.4V (solar cell, in 200 lux.)
 *  - power supply can deliver 4.8V (solar cell in full sun)
 *  - mcu Vbrownout (aka Vmin) is 1.9V
 *  - mcu Vmax is 3.6V
 *
 * Need say 2.1V on capacitor to burst the radio without brownout.
 * Need say 2.3V on capacitor to burst the work without falling below 2.1V
 *
 * Also, these levels are somewhat arbitrary, convenient to implement using PowerMonitor.
 *
 */

/*
 * Levels
 *
 * Four of them, constrained by what device implements.
 *
 * Here we also attach meaning, used by SleepSync algorithm/application.
 *
 * You can swap the middle meanings.
 * Currently the firefly algorithm uses most current for work, so reserve is below that.
 *
 * Brownout is 1.8V.
 * External voltage monitor resets at 2.0V.
 * Boot requires a small reserve.
 * Radio requires about 0.2V on capacitor.
 * Radio must not brownout mcu.
 * Work requires > 0.2V on capacitor.
 * Work may dip voltage below that needed for radio.
 * Radio is required to keep sync.
 * Work may temporarily take Vcc below needed for radio.
 */

void PowerManager::init() {
#ifdef NRF51
	adc.init();
#endif
	// powerMonitor need no init
}


// Implemented using ADC or SAADC
bool PowerManager::isPowerExcess() {
	// adc differs by family: NRF51 ADC, NRF52 SAADC
	// There is no adc device common to both families
	bool result;
#ifdef NRF52
	return powerMonitor.isVddGreaterThan2_7V();
#elif NRF51
	ADCResult value = adc.getVccProportionTo255();
	// Need to use value smaller than 0xFF? say 3.4V
	// This is fragile: must use >= since value never greater than ADC::Result3_6V
	result = (value >= ADC::Result3_6V);
#else
#error "NRF51 or NRF52 not defined"
#endif
	return result;
}


// Implemented using POFCON
bool PowerManager::isPowerAboveHigh()     { return powerMonitor.isVddGreaterThan2_7V();}
bool PowerManager::isPowerAboveMedium()   { return powerMonitor.isVddGreaterThan2_5V();}
bool PowerManager::isPowerAboveLow()      { return powerMonitor.isVddGreaterThan2_3V();}
bool PowerManager::isPowerAboveUltraLow() { return powerMonitor.isVddGreaterThan2_1V();}


VoltageRange PowerManager::getVoltageRange() {
	/*
	 * Implementation: step through levels from high to low
	 */
	VoltageRange result;
	if (isPowerExcess()) {
		result = VoltageRange::AboveExcess;
	}
	else if (isPowerAboveHigh()) {
			result = VoltageRange::HighToExcess;
		}
	else if (isPowerAboveMedium()) {
		result = VoltageRange::MediumToHigh;
	}
	else if (isPowerAboveLow()) {
		result = VoltageRange::LowToMedium;
	}
	else if (isPowerAboveUltraLow()) {
		result = VoltageRange::UltraLowToLow;
	}
	else {
		// < 2.1V, near brownout of say 1.8V
		result = VoltageRange::BelowUltraLow;
	}

	return result;
}

#ifdef OLD
bool PowerManager::isVoltageExcess() { return ;}

bool PowerManager::isVoltageHigh() {
	return ! powerManager.isVddGreaterThan2_7V()
			& powerManager.isVddGreaterThan2_5V();
}
bool PowerManager::isVoltageMedium() {
	return ! powerManager.isVddGreaterThan2_5V()
			& powerManager.isVddGreaterThan2_3V();
}
bool PowerManager::isVoltageLow() {
	return ! powerManager.isVddGreaterThan2_3V()
			& powerManager.isVddGreaterThan2_1V();
}
bool PowerManager::isVoltageUltraLow() {

	return ! powerManager.isVddGreaterThan2_1V();
}
#endif



