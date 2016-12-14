
#include "powerSupply.h"

#include "nrf.h"

void PowerSupply::enableDCDCPower(){
	NRF_POWER->DCDCEN = 1;
	// Takes effect on next system bus read (flushes ARM M4 write buffer)
}
