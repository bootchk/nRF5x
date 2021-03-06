#include <cassert>

#include "nrf.h"

#include "radio.h"
#include "../mcu.h"

/*
 * More implementation of RadioDevice is in radioConfigure.c and radioAddress.c
 *
 * These are here so that most knowledge of a kind of register manipulation is in one place.
 * I don't use comments at a higher level (Radio) to explain register manipulation, the name suffices.
 *
 */

/*
 * Clear means: clear the flag that indicates an event.
 * You start a task, which runs in the radio device.
 * The task eventually sets an event.
 * The event can trigger an interrupt.
 *
 * Per Nordic document "Migrating nrf51 to nrf52", you must read event register after clearing to flush ARM write buffer.
 */

// TODO
/*
 * If Nordic chip really manages power for radio without this,
 * another implementation is required which does nothing but set and return a local state.
 * Also, if POWER is not toggled, configuration need only be done once in the calling app.
 * And the implementation of configurePhys..() could do it only on the first call?
 *
 * For now,
 * not sure that both nrf51 and nrf52 really power down radio correctly.
 * Even if they do, it could be going to an idle state (whose power is undefined by Nordic docs)
 * and this might save more power?
 */
void RadioDevice::powerOn() { NRF_RADIO->POWER = 1; MCU::flushWriteCache(); }
void RadioDevice::powerOff() { NRF_RADIO->POWER = 0; MCU::flushWriteCache(); }
/*
 * Reset condition is power on.
 * !!! Radio registers are undefined while powered off.
 * !!! Configuration is lost when powered off.
 */
bool RadioDevice::isPowerOn() { return NRF_RADIO->POWER == 1; }


// FUTURE inline

// Events and tasks


/*
 * EndOfTransmit AND MsgReceived will BOTH set the same DISABLED event.
 * Device cannot transmit and receive at the same time.
 *
 * Since we use a shortcut, on packet done (TX or RX), radio state becoming DISABLED signifies end.
 *
 * !!! Note we enable interrupt on RX, but not on TX.
 *
 * The RX IRQHandler must clear this event, or another interrupt immediately occurs.
 *
 * The app can also stopReceive() (end receiving before any msg or after one msg), taking state to DISABLED.
 * !!! So state==DISABLED is not always MsgReceived or EndOfTransmit.
 * Must disable any interrupt on DISABLED before calling stopReceive()
 */
void RadioDevice::clearMsgReceivedEvent() {
	clearDisabledEvent();
}
void RadioDevice::clearEndTransmitEvent() {
	clearDisabledEvent();
}

void RadioDevice::startRXTask() {
	NRF_RADIO->TASKS_RXEN = 1;
	// TODO flushWriteCache everywhere
}
void RadioDevice::startTXTask() {
	NRF_RADIO->TASKS_TXEN = 1;
}

/*
 * This is general purpose (to disable both TX and RX.)
 * Note that interrupt can be enabled for EVENTS_DISABLED,
 * which will be generated.
 * Will EVENTS_DISABLED be set if already set?
 * Delay till event when TX: 4-6uS.
 * Delay till event when RX: 0uS.
 */
void RadioDevice::startDisablingTask(){
	NRF_RADIO->TASKS_DISABLE = 1;
}



void RadioDevice::clearDisabledEvent(){
	NRF_RADIO->EVENTS_DISABLED = 0;
	MCU::flushWriteCache();
}

bool RadioDevice::isDisabledState() {
	/*
	 * i.e. not busy with (in midst of) xmit or rcv
	 *
	 * Not the same as EVENTS_DISABLED:  device can be disabled without the event being set.
	 * E.G. it starts disabled.
	 */
	return NRF_RADIO->STATE == RADIO_STATE_STATE_Disabled;
}

// Is radio in middle of receiving packet?
bool RadioDevice::isReceiveInProgressEvent() {
	/*
	 * In progress is indicated by AddressEvent.
	 * Only returns true at most once per started receive.
	 *
	 * NOT by NRF_RADIO->STATE==RADIO_STATE_STATE_Rx, which just indicates radio is listening for addresses.
	 */
	uint32_t result = NRF_RADIO->EVENTS_ADDRESS;
	clearReceiveInProgressEvent();
	return result;
}

void RadioDevice::clearReceiveInProgressEvent() {
	NRF_RADIO->EVENTS_ADDRESS = 0;
	MCU::flushWriteCache();
}


bool RadioDevice::isDisabledEventSet() {
	return NRF_RADIO->EVENTS_DISABLED; // == 1
}

#ifdef USE_PACKET_DONE_FOR_EOT
/*
 * Event "END" means "end of packet" (RX or TX)
 *
 * Since we use shortcuts, state has already passed through RXIDLE or TXIDLE to DISABLED
 * But the event is still set???? is trigger for interrupt, and must be cleared.
 */
bool RadioDevice::isPacketDone() {
	return NRF_RADIO->EVENTS_END;  // == 1;
}

void RadioDevice::clearPacketDoneEvent() {
	NRF_RADIO->EVENTS_END = 0;
	MCU::flushWriteCache();
	assert(!isPacketDone());	// ensure
}

void RadioDevice::enableInterruptForPacketDoneEvent() { NRF_RADIO->INTENSET = RADIO_INTENSET_END_Msk; }
void RadioDevice::disableInterruptForPacketDoneEvent() { NRF_RADIO->INTENCLR = RADIO_INTENCLR_END_Msk; }
bool RadioDevice::isEnabledInterruptForPacketDoneEvent() { return NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk; }

#endif


/*
 * Interrupts and shortcuts
 *
 * Is there a race between END interrupt and EVENT_DISABLED when shortcut?
 *
 * !!! These just enable Radio to signal interrupt.  Must also:
 * - Nvic.enableRadioIRQ (NVIC is documented by ARM, not by Nordic)
 * - ensure PRIMASK IRQ bit is clear (IRQ enabled in mcu register)
 */

void RadioDevice::enableInterruptForDisabledEvent() { NRF_RADIO->INTENSET = RADIO_INTENSET_DISABLED_Msk; }
void RadioDevice::disableInterruptForDisabledEvent() { NRF_RADIO->INTENCLR = RADIO_INTENCLR_DISABLED_Msk; }
bool RadioDevice::isEnabledInterruptForDisabledEvent() { return NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk; }


/*
 * The radio emits events for many state transitions we are not interested in.
 * And the radio requires certain tasks to make transitions.
 * A shortcut hooks an event to a transition so that we do not need to explicitly start a task.
 * A shortcut makes for much shorter on-air times.
 *
 * !!! The state diagram also has a transition without a condition:  /Disabled from TXDISABLE to DISABLED.
 *
 * These shortcuts are:
 * - from state TXRU directly to state TX (without explicit start READY task, bypassing state TXIDLE)
 * - from state TX   directly to state DISABLED (without explicit DISABLE task, bypassing states TXIDLE and TXDISABLE)
 */
void RadioDevice::setShortcutsAvoidSomeEvents() {
	//
	// In other words, make automatic transitions in state diagram.
	NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Msk // shortcut READY event to START task
			| RADIO_SHORTS_END_DISABLE_Msk		 // shortcut END event to DISABLE task
			// next is optional.  I assume it doesn't take any more power to always sample RSSI
			| RADIO_SHORTS_ADDRESS_RSSISTART_Msk;	 // shortcut ADDRESS event to RSSISTART task

	// RadioHead nrf51
	// These shorts will make the radio transition from Ready to Start to Disable automatically
	// for both TX and RX, which makes for much shorter on-air times
	// NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos)
	//	              | (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);

}






// CRC

bool RadioDevice::isCRCValid() {
	return NRF_RADIO->CRCSTATUS == 1;	// CRCOk;
}

uint8_t RadioDevice::receivedLogicalAddress() {
	// cast lower byte of word register
	return (uint8_t) NRF_RADIO->RXMATCH;
}

unsigned int RadioDevice::receivedSignalStrength() {
	/*
	 * Assume RSSI_END event is set i.e. sample is done.
	 *
	 * Assume MSB bits read zero (no masking), only 7 LSB could be ones.
	 *
	 * Don't clear RSSI_END, we don't care.
	 */
	// Assert a shortcut is set so that
	unsigned int result = NRF_RADIO->RSSISAMPLE;
	return result;
}
