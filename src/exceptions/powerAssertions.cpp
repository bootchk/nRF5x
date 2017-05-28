#include <cassert>

#include "nrf.h"
#include "nrf_clock.h"

#include "../modules/radio.h"
#include "../drivers/adc.h"
#include "../drivers/powerComparator.h"
#include "../drivers/flashController.h"
#include "powerAssertions.h"

void assertUnusedOff();	// local
void assertNeverUsedDevicesOff();



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

	// MWU disabled
	assert(NRF_MWU->REGIONEN == 0);
#endif

	// Peripherals in reset condition

	// WDT not running
	assert(NRF_WDT->RUNSTATUS == 0);


	// TODO Not exist a register to detect power sub-mode (LOWPOWER or CONSTANT_LATENCY)
	// and Nordic docs say it defaults to low-power, but we could trigger the task here
	// NRF_POWER->TASKS_LOWPWR = 1;

	// GPIO's inputs and disconnected
	// TODO only the ones we don't use

	// DCDC power regulator disabled
	assert(NRF_POWER->DCDCEN == 0);

	// Pending flags prevent sleep even if interrupts disabled
	//NVIC_ClearPendingIRQ(FPU_IRQn);
	assert( ! NVIC_GetPendingIRQ(RADIO_IRQn));
	assert( ! NVIC_GetPendingIRQ(RTC0_IRQn));
	assert( ! NVIC_GetPendingIRQ(POWER_CLOCK_IRQn));
	assert( ! NVIC_GetPendingIRQ(ADC_IRQn));

	// TODO other, unused IRQ such as WDT, MemCU


	// Not reset for unexpected reason
	// This doesn't work in debug because the reason will be DIF
	// assert( NRF_POWER->RESETREAS == 0 );
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

	assert( PowerComparator::isDisabled());


	/*
	 * Not: assert( ADC::isDisabled());
	 * The ADC stays enabled but uses no power when not converting.
	 */

	assert( FlashController::isDisabled());

	// TODO for nrf52 can assert not in debug mode
}

void assertRadioPower() {

	assertNeverUsedDevicesOff();
}

#endif
