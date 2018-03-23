
#include "pinTask.h"

// in-line implementation using hal
#include "nrf_gpiote.h"

/*
 * This is "idx" of channel in range [0..7]
 */
#define SUNK_OFF_TASK_INDEX 0

/*
 * These are enum values, but not ints, instead offsets into a register bank.
 */
// OUT task configured hi-to-lo
#define SUNK_OFF_TASK_OFFSET  NRF_GPIOTE_TASKS_OUT_0
// CLR task i.e. set lo turns sunk pin on
#define SUNK_ON_TASK_OFFSET   NRF_GPIOTE_TASKS_CLR_0

/*
 * !!! Since mode is toggle, if ever a toggle is missed or is extra toggle, they lose coherence.
 * !!! If event is from a time, very easy to get an extra event when compare register rolls over and matches again.
 */
void PinTask::configureSunkPinTasks(
		uint32_t pin
		)
{
	/*
	 * Configuration needed for pin,
	 * but only affects the OUT task.
	 * CLR task exists without configuration.
	 */
	nrf_gpiote_task_configure(
			SUNK_OFF_TASK_INDEX,
			pin,
			NRF_GPIOTE_POLARITY_LOTOHI,	// Task off is set high
			NRF_GPIOTE_INITIAL_VALUE_HIGH	// Sunk LED is off when high
			);
}

void PinTask::enableTask() { nrf_gpiote_task_enable(SUNK_OFF_TASK_INDEX); }


void PinTask::disableTaskAndForceToInitialState(){
	nrf_gpiote_task_disable(SUNK_OFF_TASK_INDEX);

	// Ensure pin is in initial state
	nrf_gpiote_task_force(SUNK_OFF_TASK_INDEX, NRF_GPIOTE_INITIAL_VALUE_HIGH);
}


/*
 * Identify OUT register for task 0 must correspond to SUNK_OFF_TASK_INDEX.
 */
uint32_t* PinTask::getSunkOffTaskRegisterAddress() {
	/*
	 * The declared type of ...addr_get is wrong.
	 * Cast it to a pointer type.
	 */
    return (uint32_t*) (nrf_gpiote_task_addr_get(SUNK_OFF_TASK_OFFSET));
}


/*
 * Start the sunk off task by writing 1 to its register
 */
void PinTask::startSunkOffTask() {
	*getSunkOffTaskRegisterAddress() = 1;
}

void  PinTask::startSunkOnTask() {
	*(uint32_t*) (nrf_gpiote_task_addr_get(SUNK_ON_TASK_OFFSET)) = 1;
}
