
#include <cassert>

#include "radio.h"

#include "../drivers/radio/radio.h"

#include "../drivers/hfClock.h"
#include "../drivers/nvic.h"
#include "../drivers/powerSupply.h"

// Debugging
// TODO pass this into radio and only call it if non-null
// LEDService ledLogger2;


/*
 * Implementation notes:
 *
 * Interrupts are used.  Alternative is to poll (see RadioHead.)
 * Now, interrupt on receive.  FUTURE: interrupt on xmit.
 *
 * Uses "configuration registers" of the RADIO peripheral(section of the datasheet).
 * AND ALSO "task registers" of the "Peripheral Interface"(section of the datasheet)
 * to the RADIO (for tasks, events, and interrupts.)
 * See device/nrf52.h for definitions of register addresses.
 * Each task and event has its own register.
 *
 * Not keeping our own state (radio peripheral device has a state.)
 *
 * !!! Note this file has no knowledge of registers (nrf.h) , see radioLowLevel and radioConfigure.c
 */

namespace {

// protocol module owns radio device
// RadioDevice device;

// protocol module uses but doesn't own
Nvic* nvic;
PowerSupply* powerSupply;


void (*aRcvMsgCallback)() = nullptr;

// State currently just used for assertions
RadioState state;

/*
 * Buffer R/W by concurrent radio HW (volatile) using DMA.
 * No guards around buffer.
 * We pass address and length to radio HW and it does NOT write outside the buffer.
 * We also pass address and length to Serializer.
 */
volatile uint8_t radioBuffer[Radio::FixedPayloadCount];


extern "C" {

__attribute__ ((interrupt ("RADIO_IRQ")))
void
RADIO_IRQHandler()  {
	Radio::receivedEventHandler();	// relay to static C++ method
	// assert ARM is in IRQ mode and assembler will generate proper RTI instructions
	// ARM set an internal event flag will be set on RTI that must be cleared by SEV
}
}


} // namespace





HfCrystalClock* Radio::hfCrystalClock;
RadioDevice Radio::device;




void Radio::receivedEventHandler(void)
{
	// We only expect an interrupt on packet received

    if (isEventForMsgReceivedInterrupt())
    {
    	assert(state == Receiving);	// sanity

    	clearEventForMsgReceivedInterrupt();

    	// ledLogger2.toggleLED(2);	// debug: LED 2 show every receive

    	/*
    	 * Call next layer.
    	 * For SleepSyncAgent calls Sleeper::msgReceivedCallback() which sets reasonForWake
    	 */
    	assert(aRcvMsgCallback!=nullptr);
    	aRcvMsgCallback();
    }
    else
    {
        /*
         * Probable programming error.
         * We were awakened by an event other than the only enabled interrupt 'MsgReceived'
         * (which on some platforms is radio DISABLED)
         * Brownout and bus faults (DMA?) could come while mcu is sleeping.
		 * Invalid op code faults can not come while mcu is sleeping.
         */
    	// FUTURE handle more gracefully by just clearing all events???
    	// FUTURE recover by raising exception and recovering by reset?
    	assert(false);
    }
    // We don't have a queue and we don't have a callback for idle
    assert(!isEventForMsgReceivedInterrupt());	// Ensure event is clear else get another unexpected interrupt
    // assert Sleeper::reasonForWake != None
}



/*
 * Private routines that isolate which event is used for interrupt on End Of Transmission.
 * The choices are END or DISABLED.
 */
#ifdef PACKET_DONE_INTERRUPT
bool Radio::isEventForEOTInterrupt() { return device.isPacketDone(); }
void Radio::clearEventForEOTInterrupt() { device.clearPacketDoneEvent(); }
void Radio::enableInterruptForEOT() { device.enableInterruptForPacketDoneEvent(); }
void Radio::disableInterruptForEOT() { device.disableInterruptForPacketDoneEvent(); }
bool Radio::isEnabledInterruptForEOT() { return device.isEnabledInterruptForPacketDoneEvent(); }
#else
// DISABLED event
bool Radio::isEventForMsgReceivedInterrupt() {
	/*
	 * !!! The radio stays in the disabled state, even after the event is cleared.
	 * So this is not:  device.isDisabled().
	 */
	return device.isDisabledEventSet();
}
void Radio::clearEventForMsgReceivedInterrupt() { device.clearDisabledEvent(); }

/*
 * In interrupt chain, disable in two places: nvic and device
 * Assume PRIMASK is always clear to allow interrupts.
 */
void Radio::enableInterruptForMsgReceived() {
	assert(!device.isDisabledEventSet());	// else interrupt immediately???
	nvic->enableRadioIRQ();
	device.enableInterruptForDisabledEvent();
}
void Radio::disableInterruptForMsgReceived() {
	nvic->disableRadioIRQ();
	device.disableInterruptForDisabledEvent();
}
bool Radio::isEnabledInterruptForMsgReceived() {
	return device.isEnabledInterruptForDisabledEvent();	// FUTURE && nvic.isEnabledRadioIRQ();
}

void Radio::disableInterruptForEndTransmit() { device.disableInterruptForDisabledEvent(); }

bool Radio::isEnabledInterruptForEndTransmit() { return device.isEnabledInterruptForDisabledEvent(); }
#endif

/*
 * The register is read only.
 * Should only be called when packet was newly received (after an IRQ or event indicating packet done.)
 */
bool Radio::isPacketCRCValid(){
	// We don't sample RSSI
	// We don't use DAI device address match (which is a prefix of the payload)
	// We don't use RXMATCH to check which logical address was received
	// (assumed environment with few foreign 2.4Mhz devices.)
	// We do check CRC (messages destroyed by noise.)
	return device.isCRCValid();
}


#ifdef OBSOLETE
void Radio::dispatchPacketCallback() {
	// Dispatch to owner callbacks
	if ( ! wasTransmitting ) { aRcvMsgCallback(); }
	// No callback for xmit
}
#endif



void Radio::init(
		Nvic* aNvic,
		PowerSupply* aPowerSupply,
		HfCrystalClock* aHfClock
		)
{
	nvic = aNvic;
	powerSupply = aPowerSupply;
	hfCrystalClock = aHfClock;

	// Not require radio device power on

	// See Nordic docs: this saves power for certain high Vcc and large currents (e.g. radio)
	// The radio manages its use automatically?
	// There is a startup time before transmission can subsequently be started.
	powerSupply->enableDCDCPower();	// Radio device wants this enabled.

	// assert radio is configured to device reset defaults, which is non-functional.
	configurePhysicalProtocol();

	/*
	 * not ensure callback not nullptr: must set else radio might receive but not relay packet on
	 * not ensure HFXO is started, which radio needs
	 */
	assert(isConfigured());
}


void Radio::setMsgReceivedCallback(void (*onRcvMsgCallback)()){
	aRcvMsgCallback = onRcvMsgCallback;
}



void Radio::abortUse() {
	startDisableTask();
	// May be a delay until radio is disabled i.e. ready for next task
}
bool Radio::isInUse() { return ! device.isDisabledState(); }

void Radio::spinUntilDisabled() {
	/*
	 * Assert:
	 * - we started the task DISABLE
	 * - or we think is in reset state (just after RADIO->POWER toggled or POR)
	 * - or a shortcut from active state such as TX will succeed
	 * Not to be used to wait for a receive, because it may fail
	 */
	/*
	 *  Implementation: Wait until state == disabled  (not checking for event.)
	 *
	 *  See data sheet.
	 *  For (1Mbit mode, TX) to disabled delay is ~6us.
	 *  RX to disabled delay is ~0us
	 *  delay from reset (toggle POWER) to disabled is ???
	 */
	while (!device.isDisabledState()) ;
}

#ifdef OLD
void Radio::resetAndConfigure() {
	reset();
	configurePhysicalProtocol();
}



void Radio::reset() {
	// Does not require !device.isPowerOn()), it can be left enabled.

	// require Vcc > 2.1V (see note below about DCDC)

	// radio requires HFXO xtal clock, not the HFRC hf clock
	assert(hfCrystalClock->isRunning());

	// !!! toggling the bit called 'POWER' is actually just reset and does not affect power
	// The chip manages power to radio automatically.
	device.setRadioPowered(false);
	device.setRadioPowered(true);

	// Assert there is no delay: state is immediately disabled?
	// There is a delay using the DISABLE Task from RX or TX
	spinUntilReady();

	// assert if it was off, the radio and its registers are in initial state as specified by datasheet
	// i.e. it needs to be reconfigured

	// assert HFCLK is on since radio uses it

	state = Idle;

	// !!! Configuration was lost, caller must now configure it
	assert(!isEnabledInterruptForMsgReceived());
	assert(device.isDisabledState());		// after reset, initial state is DISABLED
}


void Radio::spinUntilReady() {
	// BUG: WAS disable(); spinUntilDisabled();  but then I added an assertion to disable();
	/*
	 * Wait until power supply ramps up.
	 * Until then, device registers are indeterminate?
	 *
	 * This is the way RadioHead does it.
	 * But how do you start a task when registers are indeterminate?
	 */
	device.clearDisabledEvent();
	device.startDisablingTask();
	spinUntilDisabled();
}
#endif


void Radio::shutdownDependencies() {
	// not require on; might be off already

	/*
	 * require disabled: caller must not power off without disabling
	 * because we also use the disabled state for MsgReceived (with interrupts)
	 * The docs do not make clear whether the device passes through
	 * the DISABLED state (generating an interrupt) when powered off.
	 */
	assert(!isInUse());

	/* OLD
	device.setRadioPowered(false);
	// not ensure not ready; caller must spin if necessary
	*/

	hfCrystalClock->stop();
	// assert hf RC clock resumes for other peripherals

	state = PowerOff;
	// assert HFXO off, or will be soon
	// assert the chip has powered radio off
}



#ifdef OBSOLETE
// Get address and length of buffer the radio owns.
void Radio::getBufferAddressAndLength(uint8_t** handle, uint8_t* lengthPtr) {
	*handle = staticBuffer;
	*lengthPtr = PayloadCount;
}
#endif




/*
 * Per Nordic docs, must setup DMA each xmit/rcv
 * Fixed: device always use single buffer owned by radio
 */
void Radio::setupFixedDMA() {
	device.configurePacketAddress(getBufferAddress());
}

/*
 * radioBuffer is an array, return its address by using its name on right hand side.
 */
BufferPointer Radio::getBufferAddress() { return radioBuffer; }



void Radio::transmitStaticSynchronously(){
	// ledLogger2.toggleLED(4);	// Dual purpose LED4: invalid or xmit
	disableInterruptForEndTransmit();	// spin, not interrupt

	// Lag for rampup, i.e. not on air immediately
	transmitStatic();
	// FUTURE: sleep while xmitting to save power
	spinUntilXmitComplete();
	// assert xmit is complete and device is disabled
}

void Radio::transmitStatic(){
	state = Transmitting;
	setupFixedDMA();
	startXmit();
	// not assert xmit is complete, i.e. asynchronous and non-blocking
}

void Radio::receiveStatic() {
	state = Receiving;
	setupFixedDMA();
	setupInterruptForMsgReceivedEvent();
	startRcv();
	// assert will get IRQ on message received
}

bool Radio::isReceiveInProgress() {
	return device.isReceiveInProgressEvent();
}
void Radio::spinUntilReceiveComplete() {
	// Same as:
	spinUntilXmitComplete();
}
void Radio::clearReceiveInProgress() {
	device.clearReceiveInProgressEvent();
}

#ifdef DYNAMIC
void Radio::transmit(volatile uint8_t * data, uint8_t length){
	state = Transmitting;
	setupXmitOrRcv(data, length);
	startXmit();
	// not assert xmit is complete, i.e. asynchronous and non-blocking
};


void Radio::receive(volatile uint8_t * data, uint8_t length) {
	wasTransmitting = false;
	setupXmitOrRcv(data, length);
	startRcv();
}
#endif

/*
 * Starting task is final step.
 * Start a task on the device.
 * Require device is configured, including data and DMA.
 */
void Radio::startRXTask() {
	device.clearMsgReceivedEvent();	// clear event that triggers interrupt
	device.startRXTask();
}

void Radio::startTXTask() {
	device.clearEndTransmitEvent();	// clear event we spin on
	device.startTXTask();
}

void Radio::startDisableTask() {
	assert(!isEnabledInterruptForEndTransmit());
	device.clearDisabledEvent();
	device.startDisablingTask();
	state = Idle;
}



/*
 * Usually (but not required),
 * device is configured: shortcuts, packetAddress, etc.
 * and buffer is filled.
 */
void Radio::setupInterruptForMsgReceivedEvent() {
	// Clear event before enabling, else immediate interrupt
	clearEventForMsgReceivedInterrupt();
	enableInterruptForMsgReceived();
}


#ifdef DYNAMIC
void Radio::setupXmitOrRcv(volatile uint8_t * data, uint8_t length) {
	/*
	 * Assert
	 * is configured: shortcuts, packetAddress, etc.
	 */
	device.setShortcutsAvoidSomeEvents();
	device.configurePacketAddress(data);
	//device.configurePacketLength(length);
	enableInterruptForEOT();
	clearEventForEOTInterrupt();
}
#endif





// task/event architecture, these trigger or enable radio device tasks.

void Radio::startXmit() {
	assert(device.isDisabledState());  // require, else behaviour undefined per datasheet
	startTXTask();
	// assert radio state will soon be TXRU and since shortcut, TX
}

void Radio::startRcv() {
	assert(device.isDisabledState());  // require, else behaviour undefined per datasheet
	startRXTask();
	/*
	 * assert: (since shortcuts)
	 * 1. radio state will soon be RXRU
	 * 2. ramp up delay incurred of 130us (nrf52) or 40us (nrf52)
	 * 3. radio state RXIDLE (shortcut)
	 * 4. radio state RX (shortcut)
	 */
}

void Radio::stopReceive() {
	/*
	 *  assert radio state is:
	 * RXRU : aborting before ramp-up complete
	 * or RXIDLE: on but never received start preamble signal
	 * or RX: in middle of receiving a packet
	 * or DISABLED: message was received and RX not enabled again
	 */

	disableInterruptForMsgReceived();
	//isReceiving = false;

	if (! device.isDisabledState()) {
		// was receiving and no messages received (device in state RXRU, etc. but not in state DISABLED)
		device.startDisablingTask();
		// assert radio state soon RXDISABLE and then immediately transitions to DISABLED
		spinUntilDisabled();
		// DISABLED event was just set, clear it now before we later enable interrupts for it
		device.clearMsgReceivedEvent();
	}

	/*
	 * The above checked a state returned by the radio,
	 * which experience shows can differ from the event for the state.
	 * So here we explicitly clear the event to ensure it corresponds with radio state.
	 */
	device.clearMsgReceivedEvent();

	state = Idle;

	assert(device.isDisabledState());
	assert(!device.isDisabledEventSet());	// same as MsgReceivedEvent
	assert(!device.isEnabledInterruptForDisabledEvent()); // "
	// assert no interrupts enabled from radio, for any event
}

void Radio::stopXmit() {
	// Rarely used: to abort a transmission, generally radio completes a xmit
	// FUTURE
}

void Radio::spinUntilXmitComplete() {
	// Spin mcu until xmit complete.
	// Alternatively, sleep, but radio current dominates mcu current anyway?
	// Xmit takes a few msec?

	//assert isTransmitting

	// Nothing can prevent xmit?  Even bad configuration or HFCLK not on?

	// Radio state flows (via TXDISABLE) to DISABLED.
	// Wait for DISABLED state (not the event.)
	// Here, for xmit, we do not enable interrupt on DISABLED event.
	// Since it we expect it to be quick.
	// FUTURE use interrupt on xmit.

	spinUntilDisabled();	// Disabled state means xmit done because using shortcuts
	state = Idle;
}





