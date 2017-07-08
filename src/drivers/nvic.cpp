
#include <cassert>

#include "nvic.h"

#include "nrf.h"

// Implementation uses macros defined in /components/toolchain/cmsis/include/ e.g. core_cm4.h


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



/*
 * RTC0
 */
void Nvic::enableRTC0IRQ() {
	NVIC_ClearPendingIRQ(RTC0_IRQn);
	NVIC_EnableIRQ(RTC0_IRQn);
}

void Nvic::pendRTC0Interrupt() {
	NVIC_SetPendingIRQ(RTC0_IRQn);
}




/*
 * PowerClock
 */

#ifdef NOT_USED
not supported by HAL
bool Nvic::isEnabledPowerClockIRQ(){
	return false;
	// !!! Not correct: NVIC_GetActive(POWER_CLOCK_IRQn);
}
#endif

void Nvic::enablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_EnableIRQ(POWER_CLOCK_IRQn);
	//assert(isEnabledPowerClockIRQ());
}

void Nvic::disablePowerClockIRQ(){
	NVIC_ClearPendingIRQ(POWER_CLOCK_IRQn);
	NVIC_DisableIRQ(POWER_CLOCK_IRQn);
}



/*
 *
 */

void Nvic::softResetSystem(){
	NVIC_SystemReset();
}



