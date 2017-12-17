
#include <cassert>

#include "nvic.h"


/*
 * Two implementations:
 * - incompatible with SD, using NVIC_... API
 * - compatible with SD, using sd_nvic_... API
 */
#define USING_SD 1

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


void Nvic::enableRTC1IRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(RTC1_IRQn);
	APP_ERROR_CHECK(err_code);

	/*
	 * If not set, defaults to 0, which conflicts with SD.
	 */
	err_code = sd_nvic_SetPriority(RTC1_IRQn, 7);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_EnableIRQ(RTC1_IRQn);
	APP_ERROR_CHECK(err_code);
}
void Nvic::disableRTC1IRQ() {
	ret_code_t err_code;

	err_code = sd_nvic_ClearPendingIRQ(RTC1_IRQn);
	APP_ERROR_CHECK(err_code);
	err_code = sd_nvic_DisableIRQ(RTC1_IRQn);
	APP_ERROR_CHECK(err_code);
}

void Nvic::pendRTC1Interrupt() {
	ret_code_t err_code;

	err_code = sd_nvic_SetPendingIRQ(RTC1_IRQn);
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
 * RTC1
 */
void Nvic::enableRTC1IRQ() {
	NVIC_ClearPendingIRQ(RTC1_IRQn);

	/*
	 * If not set, defaults to 0, which conflicts with SD.
	 */
	NVIC_SetPriority(RTC1_IRQn, 7);
	NVIC_EnableIRQ(RTC1_IRQn);
}
void Nvic::disableRTC1IRQ() {
	NVIC_ClearPendingIRQ(RTC1_IRQn);
	NVIC_DisableIRQ(RTC1_IRQn);
}

void Nvic::pendRTC1Interrupt() {
	NVIC_SetPendingIRQ(RTC1_IRQn);
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

