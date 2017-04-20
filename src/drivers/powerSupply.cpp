
#include "powerSupply.h"

#include "nrf.h"

void PowerSupply::enableDCDCPower(){

	// TEMP comment out, should not be enabled at less than 2.1V
	// NRF_POWER->DCDCEN = 1;
	// Takes effect on next system bus read (flushes ARM M4 write buffer)
}
