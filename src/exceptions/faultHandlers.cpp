

#include <inttypes.h>

#include "../drivers/mcu.h"
#include "../drivers/nvic.h"
#include "../modules/radio.h"


#define NDEBUG 1

namespace {


/*
 * Power off any used devices (specific to app).
 * So that the mcu might not brownout, but the app is halted (non-functional.)
 *
 * If instead, you brownout, the mcu resets and might appear to continue functioning,
 * with no external indication that a fault occurred.
 *
 * This is application specific, for the peripherals I am using.
 */


void powerOffPeripherals() {

	// TODO turn off RTC
	// LongClockTimer::stop();

	// TODO turn off GPIO's (e.g. LED)

	// Reset radio so its power off (not transceiving).  Nordic platform, uses no power.
	Radio::powerOff();
}


/*
 * A fault has occurred.
 * Attempt to record it and to stay out of brownout.
 */
void sleepForeverInLowPower() {
	// Most important to attempt to record what happened.
	// TODO record reason in flash so we can see it after repowering connected to debug probe

	// Disable all further interrupts from peripherals that might be concurrently operating.
	MCU::disableIRQ();

	// Optionally reduce power so brownout doesn't compound the fault
	powerOffPeripherals();

	while(true) {
		MCU::sleep();
	}
}


/*
 * Conditionally compile:
 * - Release mode: reset and keep trying to function
 * - Debug mode: sleep forever in low power
 */
void resetOrHalt() {
    // On fault, the system can only recover with a reset.
#ifdef NDEBUG
	// TODO we may want to leave assertions in the release version: change NDEBUG to IS_RELEASE
	// If C standard symbol NDEBUG is defined, assertions off, and this resets system instead of sleeping
    Nvic::softResetSystem();
#else
    sleepForeverInLowPower();
#endif // DEBUG
}

} // namespace


extern "C" {

void foo() {

}

#if USE_SOFT_DEVICE
/*
 * Handlers for exceptions/faults.
 * These override the defaults, which are infinite loops and quickly brownout a low-power supply.
 */


void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
	(void) id;	// unused params
	(void) pc;
	(void) info;

	resetOrHalt();
}
#endif


/*
 * Handlers for hardware faults.
 *
 * nRF51 subset (nRF52 has more)
 *
 * !!! Might not expect most of these; only HardFault can occur regardless of the application.
 * The rest might never occur depending on the design of the app:
 * - not using an OS that would generate SVC
 * - not using SysTick
 * - not using reset pin that would generate NMI
 */
/*
 * The "unused" attribute simply suppresses a compiler warning.
 * It does not guarantee that a definition overrides a weak ASM definition?
 */



void genericNMIHandler()  { resetOrHalt(); }
void genericSVCHandler(void) { resetOrHalt(); }
void genericPendSVHandler(void) { resetOrHalt(); }
void genericSysTickHandler(void) { resetOrHalt(); }
void genericHardFaultHandler(void) { resetOrHalt(); }

} // extern C
