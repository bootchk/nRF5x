
#include "instructionCache.h"

#include "nrf.h"

void InstructionCache::enable(){
#ifdef NRF52
	NRF_NVMC->ICACHECNF=0x01;
#endif
}
