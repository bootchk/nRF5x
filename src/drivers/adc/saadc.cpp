
#include "saadc.h"

// SDK
//#include "saadc/nrf_drv_saadc.h"
#include "nrf_saadc.h"





/*
 * Implementation using HAL without interrupts
 */

#define Channel0   0


namespace {


void configureSAADC() {
#ifdef NOT_USED
	nrf_drv_saadc_config_t saadc_config;

	saadc_config.low_power_mode = true;
	saadc_config.resolution = NRF_SAADC_RESOLUTION_8BIT;
	// This will make the SAADC output values from 0 (when input voltage is 0V) to 2^8=2048
	// (when input voltage is 3.6V for channel gain setting of 1/6).
	saadc_config.oversample = SAADC_OVERSAMPLE;                                           //Set oversample to 4x. This will make the SAADC output a single averaged value when the SAMPLE task is triggered 4 times.
	saadc_config.interrupt_priority = APP_IRQ_PRIORITY_LOW;                               //Set SAADC interrupt to low priority.

	err_code = nrf_drv_saadc_init(&saadc_config, saadc_callback);                         //Initialize the SAADC with configuration and callback function. The application must then implement the saadc_callback function, which will be called when SAADC interrupt is triggered
	APP_ERROR_CHECK(err_code);
#endif

	nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_8BIT);
	nrf_saadc_int_disable(NRF_SAADC_INT_ALL);
	nrf_saadc_event_clear(NRF_SAADC_EVENT_END);

	// !!! buffer must also be initialized, but later
	// nrf_saadc_buffer_init(nrf_saadc_value_t * buffer, uint32_t num)
	// not enabled
}


void configureSAADCChannel() {
	// assert idle

#ifdef NOT_USED
	// This requires nrf_saadc.c
	nrf_saadc_channel_config_t channelConfig;

	channelConfig.reference = NRF_SAADC_REFERENCE_INTERNAL;                              //Set internal reference of fixed 0.6 volts
	channelConfig.gain = NRF_SAADC_GAIN1_6;                                              //Set input gain to 1/6. The maximum SAADC input voltage is then 0.6V/(1/6)=3.6V. The single ended input range is then 0V-3.6V
	channelConfig.acq_time = NRF_SAADC_ACQTIME_3US;                                     //Set acquisition time. Set low acquisition time to enable maximum sampling frequency of 200kHz. Set high acquisition time to allow maximum source resistance up to 800 kohm, see the SAADC electrical specification in the PS.
	channelConfig.mode = NRF_SAADC_MODE_SINGLE_ENDED;                                    //Set SAADC as single ended. This means it will only have the positive pin as input, and the negative pin is shorted to ground (0V) internally.
	//Select the input pin for the channel. AIN0 pin maps to physical pin P0.02.
	channelConfig.pin_p = NRF_SAADC_INPUT_VDD;
	channelConfig.pin_n = NRF_SAADC_INPUT_DISABLED;                                      //Since the SAADC is single ended, the negative pin is disabled. The negative pin is shorted to ground internally.
	channelConfig.resistor_p = NRF_SAADC_RESISTOR_DISABLED;                              //Disable pullup resistor on the input pin
	channelConfig.resistor_n = NRF_SAADC_RESISTOR_DISABLED;

	nrf_saadc_channel_init(Channel0, &channelConfig);
#else
	// This is in-lined
	NRF_SAADC->CH[Channel0].CONFIG =
	            ((NRF_SAADC_RESISTOR_DISABLED   << SAADC_CH_CONFIG_RESP_Pos)   & SAADC_CH_CONFIG_RESP_Msk)
	            | ((NRF_SAADC_RESISTOR_DISABLED << SAADC_CH_CONFIG_RESN_Pos)   & SAADC_CH_CONFIG_RESN_Msk)
	            | ((NRF_SAADC_GAIN1_6       << SAADC_CH_CONFIG_GAIN_Pos)   & SAADC_CH_CONFIG_GAIN_Msk)
	            | ((NRF_SAADC_REFERENCE_INTERNAL  << SAADC_CH_CONFIG_REFSEL_Pos) & SAADC_CH_CONFIG_REFSEL_Msk)
	            | ((NRF_SAADC_ACQTIME_3US   << SAADC_CH_CONFIG_TACQ_Pos)   & SAADC_CH_CONFIG_TACQ_Msk)
	            | ((NRF_SAADC_MODE_SINGLE_ENDED       << SAADC_CH_CONFIG_MODE_Pos)   & SAADC_CH_CONFIG_MODE_Msk)
	            | ((NRF_SAADC_BURST_ENABLED      << SAADC_CH_CONFIG_BURST_Pos)  & SAADC_CH_CONFIG_BURST_Msk);

	nrf_saadc_channel_input_set(Channel0, NRF_SAADC_INPUT_VDD, NRF_SAADC_INPUT_DISABLED);
#endif
}


unsigned int convert8BitResultToPercentageOf3_6V(unsigned int result)  {
	return (result * 100) / 255;
}

} // namespace




void VccMonitor::init() {
	configureSAADC();
	configureSAADCChannel();
	/*
	 * ???
	 * enable means: allow conversions to be started.
	 * Will be low-power while not started.
	 * Disable means: allow configuration to be done.
	 * No need to disable to achieve low power.
	 */
	nrf_saadc_enable();
}


unsigned int VccMonitor::getVccProportionTo255() {
	// saadc do DMA to stack address
	nrf_saadc_value_t result;	// signed short 16-bit

	nrf_saadc_buffer_init(&result, 1);
	nrf_saadc_task_trigger(NRF_SAADC_TASK_START);
	nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);
	// blocking
	while (0 == nrf_saadc_event_check(NRF_SAADC_EVENT_END) ) {}

	// Stop so low-power.
	nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
	return result;

	// convert8BitResultToPercentageOf3_6V((unsigned int) result);	// return value off stack
}
