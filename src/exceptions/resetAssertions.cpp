#include <cassert>

#include "nrf.h"

#include "resetAssertions.h"


void assertNoResetsOccurred() {
	assert( NRF_POWER->RESETREAS == 0 );

}


void clearResetReason() {
	// Writing 1's clears bits
	NRF_POWER->RESETREAS = 0xFFFFFFFF;
}
