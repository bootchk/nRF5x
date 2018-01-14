
#include <cassert>

#include "nvic.h"


/*
 * Two implementations:
 * - incompatible with SD, using NVIC_... API
 * - compatible with SD, using sd_nvic_... API
 */

// only need CMSIS defs, and nrf.h seems to include them
// Implementation uses macros defined in /components/toolchain/cmsis/include/ e.g. core_cm4.h
#include "nrf.h"

#include "hwConfig.h"




/*
 * Radio
 */
void Nvic::enableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);
}

void Nvic::disableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_DisableIRQ(RADIO_IRQn);
}

#ifdef FUTURE
not supported by HAL
bool Nvic::isEnabledRadioIRQ() {
	return NVIC_GetEnableIRQ(RADIO_IRQn);
}


bool Nvic::isEnabledPowerClockIRQ(){
	return false;
	// !!! Not correct: NVIC_GetActive(POWER_CLOCK_IRQn);
}
#endif

/*
 * RTCx
 */
void Nvic::enableLFTimerIRQ() {
	NVIC_ClearPendingIRQ(LFTimerRTCIRQ);

	/*
	 * If not set, defaults to 0, which conflicts with SD.
	 */
	NVIC_SetPriority(LFTimerRTCIRQ, 7);
	NVIC_EnableIRQ(LFTimerRTCIRQ);
}
void Nvic::disableLFTimerIRQ() {
	NVIC_ClearPendingIRQ(LFTimerRTCIRQ);
	NVIC_DisableIRQ(LFTimerRTCIRQ);
}

void Nvic::pendLFTimerInterrupt() {
	NVIC_SetPendingIRQ(LFTimerRTCIRQ);
}




/*
 * PowerClock
 *
 * !!! PowerClockIRQ also used by SD.
 * If using SD, must disable interrupts before enabling SD.
 */

void Nvic::enablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_EnableIRQ(POWER_CLOCK_IRQn);
	//assert(isEnabledPowerClockIRQ());
}

void Nvic::disablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_DisableIRQ(POWER_CLOCK_IRQn);
}




void Nvic::softResetSystem(){
	NVIC_SystemReset();
}

