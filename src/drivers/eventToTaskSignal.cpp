
#include "eventToTaskSignal.h"

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
