
#include "powerSupply.h"

#include "nrf.h"

void DCDCPowerSupply::enable(){
	// should not be enabled at less than 2.1V
	NRF_POWER->DCDCEN = 1;
	// Takes effect on next system bus read (flushes ARM M4 write buffer)
}

void DCDCPowerSupply::disable(){
	NRF_POWER->DCDCEN = 0;
	// Takes effect on next system bus read (flushes ARM M4 write buffer)
}
