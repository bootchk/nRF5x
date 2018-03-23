
#include "eventToTaskSignal.h"

/*
 * Uses hal.
 * Safe w/ SD as long as channel<19 and group<3
 */
#include "nrf_ppi.h"


// hardcoded to channel 0


void EventToTaskSignal::connect(uint32_t * eventAddress, uint32_t * taskAddress) {
	// TODO bug in documentation?? requires cast
	nrf_ppi_channel_endpoint_setup(
			NRF_PPI_CHANNEL0,
			(uint32_t) eventAddress,
			(uint32_t) taskAddress);

	nrf_ppi_channel_enable(NRF_PPI_CHANNEL0);
}

void EventToTaskSignal::connectOneShot(uint32_t * eventAddress, uint32_t * taskAddress) {

	// connection user wants
	connect(eventAddress, taskAddress);

	// Fork channel to disable self's group
	nrf_ppi_fork_endpoint_setup(NRF_PPI_CHANNEL0,
	                            nrf_ppi_task_address_get(NRF_PPI_TASK_CHG0_DIS));

	// channel 0 in group 0
	nrf_ppi_channel_include_in_group(NRF_PPI_CHANNEL0, NRF_PPI_CHANNEL_GROUP0);

	// enable channel group 0
	enableOneShot();
}

void EventToTaskSignal::enableOneShot() {
	nrf_ppi_group_enable(NRF_PPI_CHANNEL_GROUP0);
}
