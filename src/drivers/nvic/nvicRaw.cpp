
#include <drivers/nvic/nvicRaw.h>
#include <cassert>

#include "nrf.h"

#include "../hwConfig.h"




/*
 * Radio
 */
void NvicRaw::enableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);
}

void NvicRaw::disableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_DisableIRQ(RADIO_IRQn);
}

#ifdef FUTURE
not supported by HAL
bool NvicRaw::isEnabledRadioIRQ() {
	return NVIC_GetEnableIRQ(RADIO_IRQn);
}


bool NvicRaw::isEnabledPowerClockIRQ(){
	return false;
	// !!! Not correct: NVIC_GetActive(POWER_CLOCK_IRQn);
}
#endif

/*
 * RTCx
 */
void NvicRaw::enableLFTimerIRQ() {
	NVIC_ClearPendingIRQ(LFTimerRTCIRQ);

	/*
	 * If not set, defaults to 0, which conflicts with SD.
	 */
	NVIC_SetPriority(LFTimerRTCIRQ, 7);
	NVIC_EnableIRQ(LFTimerRTCIRQ);
}
void NvicRaw::disableLFTimerIRQ() {
	NVIC_ClearPendingIRQ(LFTimerRTCIRQ);
	NVIC_DisableIRQ(LFTimerRTCIRQ);
}

void NvicRaw::pendLFTimerInterrupt() {
	NVIC_SetPendingIRQ(LFTimerRTCIRQ);
}




/*
 * PowerClock
 *
 * !!! PowerClockIRQ also used by SD.
 * If using SD, must disable interrupts before enabling SD.
 */

void NvicRaw::enablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_EnableIRQ(POWER_CLOCK_IRQn);
	//assert(isEnabledPowerClockIRQ());
}

void NvicRaw::disablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_DisableIRQ(POWER_CLOCK_IRQn);
}




void NvicRaw::softResetSystem(){
	NVIC_SystemReset();
}

