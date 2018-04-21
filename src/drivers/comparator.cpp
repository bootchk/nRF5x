
#include "comparator.h"

// NRF SDK
#include "nrf_comp.h"


namespace {

nrf_comp_ref_t convertRefVolts(ComparatorReferenceVolts refVolts) {
	nrf_comp_ref_t result;
	switch (refVolts) {
	case ComparatorReferenceVolts::V1_2: result = ((nrf_comp_ref_t) COMP_REFSEL_REFSEL_Int1V2);  break;
	case ComparatorReferenceVolts::V1_8: result = ((nrf_comp_ref_t) COMP_REFSEL_REFSEL_Int1V8);  break;
	case ComparatorReferenceVolts::V2_4: result = ((nrf_comp_ref_t) COMP_REFSEL_REFSEL_Int2V4);  break;
	}
	return result;
}


}	// namespace


bool Comparator::initCompareAndShutdown(ComparatorReferenceVolts refVolts) {
	// init

	// Single ended mode, i.e. use one analog pin compared to internal reference
	nrf_comp_main_mode_set((nrf_comp_main_mode_t)COMP_MODE_MAIN_SE);
	// Low power, slow read
	nrf_comp_speed_mode_set((nrf_comp_sp_mode_t) COMP_MODE_SP_High);	// Low


	// Select internal reference voltage (VBG), from given enum value
	nrf_comp_ref_set(convertRefVolts(refVolts));

	// hardcoded analog pin 0
	nrf_comp_input_select(NRF_COMP_INPUT_0);

	// !!! Thresholds must be set even though not using crossing events.
	nrf_comp_th_t thresholdConfig;
	thresholdConfig.th_up = 0x30;	// 100% of reference voltage, 0x3F ==6-bits of 1's == decimal 63
	thresholdConfig.th_down = 0x20;
	nrf_comp_th_set(thresholdConfig);

	// config hysteresis
	// docs say not needed in single-ended mode
	// NRF_LPCOMP->HYST=1;

	//start
	nrf_comp_enable();
	nrf_comp_task_trigger(NRF_COMP_TASK_START);
	while(!nrf_comp_event_check(NRF_COMP_EVENT_READY));	// spins typically 50uSec

	// must be clear so SAMPLE task can set it
	nrf_comp_event_clear(NRF_COMP_EVENT_READY);

	nrf_comp_task_trigger(NRF_COMP_TASK_SAMPLE);
	// READY event is NOT set when sample is ready.
	// See propagation delay for

	unsigned int result = nrf_comp_result_get();

	bool isAbove;
	if (result == 0 ) isAbove = false;
	else              isAbove = true;


	// We don't care about threshold crossing events

	nrf_comp_disable();
	// assert READY event will soon be clear
	return isAbove;
}