
#include "flashController.h"
#include <nrf.h>

void FlashController::enableWrite(){
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
}


void FlashController::disableWrite(){
	NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos;
	while (NRF_NVMC->READY == NVMC_READY_READY_Busy){}
}

bool FlashController::isDisabled() {
	return (NRF_NVMC->CONFIG == 0); // read-only state
}
