
#include <cassert>
#include <inttypes.h>
#include <nrf.h>
#include "adc.h"

/*
 * Implementation notes
 *
 * Does NOT use HAL because HAL does not seem to support input is Vcc scaled.
 *
 * Uses raw manipulation of registers.
 *
 * 8-bit resolution throughout
 * Compare 1/3Vcc to 1.2V reference band gap (VBG)
 */

#define NRF51	// TEMP

#ifdef NRF51

namespace {

void enableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
}

#ifdef NOTUSED
void disableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
}
#endif

/*
 * An alternative is to clear the END event, and spin on it
 */
bool isADCBusy() {
	return ( (NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) == ADC_BUSY_BUSY_Msk);
}


void configureADCReadOneThirdVccReference1_2VInternal() {
	NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos) |
		              (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
		              (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) |
		              (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
		              (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);
}

void spinUntilConversionDone() {
	while ( isADCBusy() ) {};
}

uint16_t readADC() {

	// Not selecting an input (not using the input multiplexer.)

	NRF_ADC->TASKS_START = 1;

	spinUntilConversionDone();

	uint16_t result;
	result = (uint16_t) NRF_ADC->RESULT; // 8 bits valid
	// Even if Vcc greater than 3.6V, result still less than 0xFF (for 8-bit resolution)
	assert(result<=ADC::Result3_6V);
	return result;
	// assert ADC still enabled but not busy.  It will power down automatically.
}


}	//namespace


void ADC::init() {
	configureADCReadOneThirdVccReference1_2VInternal();
	enableADC();	// acquire input channel

	// Configuration and input acq permanent.  Only using ADC for one purpose.
}



bool ADC::isDisabled() {
	return NRF_ADC->ENABLE == ADC_ENABLE_ENABLE_Disabled;
}


ADCResult ADC::getVccProportionTo255(){
	//uint32_t foo = nrf_adc_result_get();

	uint16_t result = readADC();	// busy wait

	// One post says it does not save current to disable

	/*
	 * For 10-bit resolution:
	 * Result/1024 is proportion of Vcc to 3.6V
	 * If result is 1024, Vcc is greater than or equal to 3.6V
	 */
	return result;
}

#endif	// NRF51
