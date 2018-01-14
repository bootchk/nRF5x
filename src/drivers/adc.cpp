
#include <cassert>
#include <nrf.h>

#include "adc.h"

/*
 * Implementation notes
 *
 * Using HAL
 * (OLD implementation uses raw registers but doesn't seem to work.)
 *
 * 8-bit resolution throughout
 * Compare 1/3Vcc to 1.2V reference band gap (VBG)
 */


#ifdef NRF51

#include <nrf_adc.h>	// HAL


namespace {

bool isConfigured = false;

nrf_adc_config_t adcConfigParams;


#ifdef OLD
void enableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
}

#ifdef NOTUSED
void disableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
}
#endif


bool isADCBusy() {
	return ( (NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) == ADC_BUSY_BUSY_Msk);
}
#endif


void configureADCReadOneThirdVccReference1_2VInternal() {
#ifdef OLD
	NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos) |
		              (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
		              (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) |
		              (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
		              (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);
#else
	adcConfigParams.resolution = NRF_ADC_CONFIG_RES_8BIT;
	adcConfigParams.scaling    = NRF_ADC_CONFIG_SCALING_SUPPLY_ONE_THIRD;
	adcConfigParams.reference  = NRF_ADC_CONFIG_REF_VBG;
	nrf_adc_configure(&adcConfigParams);
	// ADC is disabled
	nrf_adc_enable();
#endif
}

#ifdef OLD
/*
 * Alternatives:
 * -clear the END event before this, and spin on it
 * -spin on busy
 */
void spinUntilNotBusy() {
	while ( isADCBusy() ) {};
}
void spinUntilConversionDone() {
	while ( ! NRF_ADC->EVENTS_END ) {};
}
#endif


ADCResult readADC() {

#ifdef OLD
	assert(isConfigured);
	assert(! ADC::isDisabled());
	/*
	 * We leave ADC enabled.
	 * enable() only acquires the input (multiplexor)
	 * If another task might be using the ADC on another input, need to select input channel here.
	 */

	NRF_ADC->EVENTS_END = 0;
	NRF_ADC->TASKS_START = 1;
	spinUntilConversionDone();

	ADCResult result;
	result = NRF_ADC->RESULT; // 8 bits valid
	// Even if Vcc greater than 3.6V, result still less than 0xFF (for 8-bit resolution)
	assert(result<=ADC::Result3_6V);
	assert(result>0);	// sanity, for purpose reading Vcc
	return result;
	// assert ADC still enabled but not busy.  It will power down automatically.
#else
	return nrf_adc_convert_single(NRF_ADC_CONFIG_INPUT_DISABLED);
#endif
}


}	//namespace



bool ADC::isDisabled() {
	return NRF_ADC->ENABLE == ADC_ENABLE_ENABLE_Disabled;
}


void ADC::init() {
#ifdef OLD
	enableADC();	// Enable use, or acquire input channel(which defaults to none)?
	spinUntilNotBusy();	// Can't configure while busy
	configureADCReadOneThirdVccReference1_2VInternal();
	// Configuration and input acq permanent.  Only using ADC for one purpose.
#else
	configureADCReadOneThirdVccReference1_2VInternal();
#endif
	isConfigured = true;
}






ADCResult ADC::getVccProportionTo255(){
	//uint32_t foo = nrf_adc_result_get();

	ADCResult result = readADC();	// busy wait

	// One post says it does not save current to disable

	/*
	 * For 10-bit resolution:
	 * Result/1024 is proportion of Vcc to 3.6V
	 * If result is 1024, Vcc is greater than or equal to 3.6V
	 */
	return result;
}

#endif	// NRF51
