
#include "pinTask.h"

// in-line implementation
#include "nrf_gpiote.h"

#define MY_TASK_INDEX 0


void PinTask::configureTaskOnPinToToggleInitiallyHigh(
		uint32_t pin
		)
{
	nrf_gpiote_task_configure(
			MY_TASK_INDEX,
			pin,
			NRF_GPIOTE_POLARITY_TOGGLE,
			NRF_GPIOTE_INITIAL_VALUE_HIGH	// Sunk LED is off when high
			);
}

void PinTask::enableTask() { nrf_gpiote_task_enable(MY_TASK_INDEX); }

void PinTask::disableTaskAndForceToInitialState(){
	nrf_gpiote_task_disable(MY_TASK_INDEX);

	// Ensure pin is in initial state
	nrf_gpiote_task_force(MY_TASK_INDEX, NRF_GPIOTE_INITIAL_VALUE_HIGH);
}


/*
 * Identify OUT register for task 0 must correspond to MY_TASK_INDEX.
 */
uint32_t PinTask::getTaskRegisterAddress() {
    return  nrf_gpiote_task_addr_get(NRF_GPIOTE_TASKS_OUT_0);
}
