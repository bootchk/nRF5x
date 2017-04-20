
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
 * !!! Note using a placeholderTimer, which occasionally wakes us and does not set reasonForWake.
 */
void MCU::sleep() {

	// Make sure any pending events are cleared
	__SEV();
	__WFE();	// Since internal event flag is set, this clears it without sleeping
	__WFE();	// This should actually sleep until the next event.

	// For more information on the WFE - SEV - WFE sequence, please refer to the following Devzone article:
	// https://devzone.nordicsemi.com/index.php/how-do-you-put-the-nrf51822-chip-to-sleep#reply-1589

	// There is conflicting advice about the proper order.  This order seems to work.
}


void MCU::flushWriteCache() {
	// Implementation: read any address, which flushes write cache.
	(void) NRF_POWER->POFCON;
}


void MCU::enableInstructionCache(){
#ifdef NRF52
	NRF_NVMC->ICACHECNF=0x01;
#endif
}


void MCU::disableIRQ(){
	__disable_irq();
}
