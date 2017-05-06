#include <cassert>

#include "nrf.h"
#include "nrf_clock.h"

#include "../modules/radio.h"
#include "powerAssertions.h"

void assertUnusedOff();	// local




/*
 * For debugging, assert conditions proper to allow ultra low power.
 * The nrf chip will power down peripherals and clocks not used to achieve low power.
 *
 * Do not call when radio is being used: HF XTAL source and radio enabled.
 *
 * Some coded w/o using drivers, i.e. not portable
 *
 * PowerComparator??
 * TODO GPIO's configured disconnected
 * TODO no interrupts/events pending that would prevent sleep
 */

/*
 * Peripherals we never use at all.
 */
void assertNeverUsedDevicesOff() {

#ifdef NRF52
	// nrf52 FPU is disabled.
	assert(SCB->CPACR == 0);

	// FPU exception not set, errata 87
	#define FPU_EXCEPTION_MASK 0x0000009F
	assert(! (__get_FPSCR()  & FPU_EXCEPTION_MASK) );

	// FPU interrupt not pending
	assert( ! NVIC_GetPendingIRQ(FPU_IRQn));
#endif

	// Peripherals in reset condition

	// Not writing to FLASH
	assert(NRF_NVMC->CONFIG == 0);
	// WDT not running
	assert(NRF_WDT->RUNSTATUS == 0);
#ifdef NRF52
	// MWU disabled
	assert(NRF_MWU->REGIONEN == 0);
#endif

	// TODO Not exist a register to detect power sub-mode (LOWPOWER or CONSTANT_LATENCY)
	// and Nordic docs say it defaults to low-power, but we could trigger the task here
	// NRF_POWER->TASKS_LOWPWR = 1;

	// GPIO's inputs and disconnected
	// TODO only the ones we don't use

	// DCDC power regulator disabled
	assert(NRF_POWER->DCDCEN == 0);


	//NVIC_ClearPendingIRQ(FPU_IRQn);
}


#ifdef NDEBUG
void assertUltraLowPower() { return; }
void assertRadioPower() { return; }
#else

/*
 * All peripherals except RTC and LFXO off.
 * Including peripherals we use, but should be inactive at time of call: radio and its HFXO.
 */
void assertUltraLowPower() {

	assertNeverUsedDevicesOff();

	assert(!Radio::isInUse());

	assert(! HfCrystalClock::isRunning());
}

void assertRadioPower() {

	assertNeverUsedDevicesOff();
}

#endif

