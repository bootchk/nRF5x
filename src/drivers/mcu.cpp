
#include <cassert>

#include "mcu.h"

#include "nrf.h"	// SEV, WEV



/*
 * nrf52:
 * - will not wake from "system off" by a timer (only reset or GPIO pin change.)
 * - in "system on", current is ~3uA == Ion + Irtc + Ix32k
 * No worry about RAM retention in "system on"
 * Here "system" means mcu.
 * Internal event flag is NOT same e.g. RADIO.EVENT_DONE.
 * Internal event flag is set by RTI in ISR.
 *
 */





/*
 * Implementation notes:
 *
 * If an event has already occurred, this will not sleep.
 * Caller must be prepared  for this situation.
 *
 * SEV, WFE, WFE is NOT correct: if there is already an event,
 * it would sleep despite the event having occurred.
 * There is conflicting advice about the proper order.
 * The WFE, SEV, WFE order seems to be most often recommended/used.
 */
void MCU::sleepUntilEvent() {
	// If no event is set, sleep until the next event, at which time the event register will be set.
	// If event register is already set, clear it.
	__WFE();
	// Assert event register MIGHT be set

	/*
	 * This sequence is the only way to clear event register:
	 * - insure it is set
	 * - wait, which since is set, will clear it
	 */
	__SEV();
	__WFE();	// Since internal event flag is set, this clears it without sleeping

	// For more information on the WFE - SEV - WFE sequence, please refer to the following Devzone article:
	// https://devzone.nordicsemi.com/index.php/how-do-you-put-the-nrf51822-chip-to-sleep#reply-1589
}

void MCU::sleepUntilInterrupt() {
	// All events we are interested in have interrupts enabled
	__WFI();
}


void MCU::clearEventRegister() {
	__SEV();
	__WFE();	// Since internal event flag is set, this clears it without sleeping
}

/*
 * Must not be optimized out.
 */
__attribute__((optimize("O0")))
void MCU::flushWriteCache() {
	// Implementation: read any IO mapped address, which flushes write buffer on IO bus nrf52
	(void) NRF_POWER->POFCON;
}


void MCU::enableInstructionCache(){
#ifdef NRF52832_XXAA
	NRF_NVMC->ICACHECNF=0x01;
#else
	// NRF51 series and NRF52810 don't have this register
    #pragma message( "No instruction cache enable." )
#endif
}


void MCU::disableIRQ(){
	__disable_irq();
}


bool MCU::isDebugMode() {
#ifdef NRF52_SERIES
	/*
	 * Not sure this works.
	 * If not debug mode, are DWT registers readable?
	 */
	// Debug mode has an instruction counter running
	return (DWT->CYCCNT != 0);
#else
	// nrf51 not possible to know in software
	return false;
#endif
}


/*
 * RESETREAS register is cumulative through resetpin, soft, dog, lockup, and wake-from-off resets.
 * POR and Brownout resets clear it.
 *
 * If a har
 */
bool MCU::isResetReason() {
	return (NRF_POWER->RESETREAS != 0);
}

void MCU::clearResetReason() {
	// Writing 1's clears bits
	NRF_POWER->RESETREAS = 0xFFFFFFFF;
}

void MCU::breakIntoDebuggerOrHardfault() {
	__asm("BKPT #0\n") ;
}
