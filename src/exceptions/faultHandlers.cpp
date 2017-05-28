

#include <inttypes.h>

#include "../drivers/mcu.h"
#include "../drivers/nvic.h"
#include "../modules/radio.h"
#include "../drivers/customFlash.h"

#include "faultHandlers.h"


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

	// Make radio not transceiving (chip will power it off.)
	Radio::abortUse();
	// Also power off radio's dependencies (HFXO)
	Radio::shutdownDependencies();
}



/*
 * Conditionally compile:
 * - Release mode: reset and keep trying to function
 * - Debug mode: sleep forever in low power
 */
__attribute__((noreturn))
void resetOrHalt() {
    // On fault, the system can only recover with a reset.
#ifdef NDEBUG
	// TODO we may want to leave assertions in the release version: change NDEBUG to IS_RELEASE
	// If C standard symbol NDEBUG is defined, assertions off, and this resets system instead of sleeping.
    Nvic::softResetSystem();
#else
    sleepForeverInLowPower();
#endif // DEBUG
}

} // namespace





extern "C" {

/*
 * A fault has occurred.
 * Try to record it and to stay out of brownout.
 */
__attribute__((noreturn))
void sleepForeverInLowPower() {
	// Most important to try record what happened.
	// The caller should do that before calling this.

	// Disable all further interrupts from peripherals that might be concurrently operating.
	MCU::disableIRQ();

	// Optionally reduce power so brownout doesn't compound the fault
	powerOffPeripherals();

	// Since there are no peripherals and no timer, this will sleep in very low power
	// The only thing that can wake it is an interrupt signal on a pin.
	while(true) {
		MCU::sleep();
	}
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

__attribute__((noreturn))
void genericHardFaultHandler(void) {
	CustomFlash::writeZeroAtIndex(HardFaultFlagIndex);
	resetOrHalt();
}

__attribute__((noreturn))
void genericExitFaultHandler(void) {
	CustomFlash::writeZeroAtIndex(ExitFaultFlagIndex);
	resetOrHalt();
}

void genericAssertionFaultHandler(const char* functionName, int lineNumber){
	CustomFlash::writeIntAtIndex(LineNumberFlagIndex, lineNumber);
	CustomFlash::copyStringToFlash(functionName);
	resetOrHalt();
}

} // extern C
