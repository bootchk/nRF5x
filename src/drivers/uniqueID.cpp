
#include "uniqueID.h"

#include <nrf.h>

/*
 * unique ID burned at factory
 * 6 bytes
 */


// FUTURE use a 48-bit bit field


uint64_t deviceID() {
	/*
	 * NRF_FICR->DEVICEADDR[] is array of 32-bit words.
	 * NRF_FICR->DEVICEADDR yields type (unit32_t*)
	 * Cast: (uint64_t*) NRF_FICR->DEVICEADDR yields type (unit64_t*)
	 * Dereferencing: *(uint64_t*) NRF_FICR->DEVICEADDR yields type uint64_t
	 *
	 * Nordic doc asserts upper two bytes read all ones.
	 */
	uint64_t result = *((uint64_t*) NRF_FICR->DEVICEADDR);
	return result;
}


