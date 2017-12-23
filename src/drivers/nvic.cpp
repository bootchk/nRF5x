
#include <cassert>

#include "nvic.h"


/*
 * Two implementations:
 * - incompatible with SD, using NVIC_... API
 * - compatible with SD, using sd_nvic_... API
 */
#define USING_SD 1

/*
 * Configure which RTCx to use
 */
// use RTC2
#define LFTimerRTCIRQ     RTC2_IRQn

#ifdef USING_SD


/*
 * Insure you get components/softdevice/sxxx/headers/nrf_nvic.h
 * and not components/drivers_nrf/nrf_soc_nosd/nrf_nvic.h.
 * The latter will not define (possibly inline) the functions in the library,
 * and you get link error "undefined reference to sd_nrf_<foo>
 */
#include "s132/headers/nrf_nvic.h"	// components/softdevice/s132/headers
#include "app_error.h"	// sd function return error code



void Nvic::enableRadioIRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(RADIO_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_EnableIRQ(RADIO_IRQn);
	APP_ERROR_CHECK(err_code);
}

void Nvic::disableRadioIRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(RADIO_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_DisableIRQ(RADIO_IRQn);
	APP_ERROR_CHECK(err_code);
}


void Nvic::enableLFTimerIRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(LFTimerRTCIRQ);
	APP_ERROR_CHECK(err_code);

	/*
	 * If not set, defaults to 0, which conflicts with SD.
	 */
	err_code = sd_nvic_SetPriority(LFTimerRTCIRQ, 7);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_EnableIRQ(LFTimerRTCIRQ);
	APP_ERROR_CHECK(err_code);
}

void Nvic::disableLFTimerIRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(LFTimerRTCIRQ);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_DisableIRQ(LFTimerRTCIRQ);
	APP_ERROR_CHECK(err_code);
}

void Nvic::pendLFTimerInterrupt() {
	ret_code_t err_code;

	err_code = sd_nvic_SetPendingIRQ(LFTimerRTCIRQ);
	APP_ERROR_CHECK(err_code);
}

void Nvic::enablePowerClockIRQ(){
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(POWER_CLOCK_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_EnableIRQ(POWER_CLOCK_IRQn);
	APP_ERROR_CHECK(err_code);
	//assert(isEnabledPowerClockIRQ());
}

void Nvic::disablePowerClockIRQ(){
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(POWER_CLOCK_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_DisableIRQ(POWER_CLOCK_IRQn);
	APP_ERROR_CHECK(err_code);
}


void Nvic::softResetSystem(){
	ret_code_t err_code;

	err_code = sd_nvic_SystemReset();
	APP_ERROR_CHECK(err_code);
}



#else


// only need CMSIS defs, and nrf.h seems to include them
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



/*
 *
 */

void Nvic::softResetSystem(){
	NVIC_SystemReset();
}

#endif

