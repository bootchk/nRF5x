
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

namespace {

void enableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled;
}

void disableADC() {
	NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Disabled;
}

void configureADCReadOneThirdVccReference1_2VInternal() {
	NRF_ADC->CONFIG = (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos) |
		              (ADC_CONFIG_INPSEL_SupplyOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) |
		              (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) |
		              (ADC_CONFIG_PSEL_Disabled << ADC_CONFIG_PSEL_Pos) |
		              (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos);
}

uint16_t readADC() {
	NRF_ADC->TASKS_START = 1;
	while (((NRF_ADC->BUSY & ADC_BUSY_BUSY_Msk) >> ADC_BUSY_BUSY_Pos) == ADC_BUSY_BUSY_Busy) {};
	return (uint16_t)NRF_ADC->RESULT; // 8 bit
}
}


int ADC::getVccProportionTo255(){
	//uint32_t foo = nrf_adc_result_get();

	enableADC();
	configureADCReadOneThirdVccReference1_2VInternal();
	uint16_t result = readADC();	// busy wait

	// One post says it does not save current to disable
	disableADC();

	// Result/1024 is proportion of Vcc to 3.6V
	// If result is 1024, Vcc is greater than or equal to 3.6V
	return result;
};
