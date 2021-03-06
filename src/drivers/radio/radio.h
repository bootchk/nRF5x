
#pragma once

#include <inttypes.h>

// radio driver configures HFClock
#include "../oscillators/hfClock.h"

#include "types.h"	// RadioBufferPointer


/*
 * Low-level driver for radio peripheral
 *
 * Understands registers, etc.
 * Does not understand any wireless protocol.
 * Provides services for wireless protocols.
 *
 * Singleton, all static class methods.
 */
class RadioDevice {

public:
	// Configuration: see notes in radioConfigure.c

	static void configureFixedFrequency(uint8_t frequencyIndex);
	static void configureFixedLogicalAddress();
	static void configureNetworkAddressPool();
	static void configureShortCRC();
	static void configureMediumCRC();
	static void configureStaticPacketFormat(const uint8_t, const uint8_t );
	static void configureWhiteningOn();	// Must follow configureStaticPacketFormat()
	static void configureWhiteningSeed(int);
	static void configureMegaBitrate(unsigned int baud);
	static void configureFastRampUp();

	static void configureXmitPower(int8_t dBm);
	static int8_t getXmitPower();

	// value that indicates has been configured (not in reset configuration)
	static uint32_t configurationSignature();

private:
	static void configureShortCRCLength();
	static void configureShortCRCGeneratorInit();
	static void configureShortCRCPolynomialForShortData();
	static void configureStaticOnAirPacketFormat();
	static void configureStaticPayloadFormat(const uint8_t PayloadCount, const uint8_t AddressLength);



	// overloaded
	static void setFirstNetworkAddressInPool(const uint8_t* address, const uint8_t len);
	static void setFirstNetworkAddressInPool();

public:
	static void configurePacketAddress(RadioBufferPointer data);
	static void configurePacketLengthDynamic(uint8_t length);



	// Getters of configuration
	static uint32_t frequency();


	static bool isCRCValid();
	static uint8_t receivedLogicalAddress();

	/*
	 * ??? Might not be necessary for Nordic.
	 * Anyway, Radio module requires this.
	 * Nordic: NRF_RADIO->POWER is just for reset and does not actually power off/on the radio.
	 * The PowerManager powers on the radio automatically as necessary (when the radio is InUse.)
	 */
	static void powerOn();
	static void powerOff();
	static bool isPowerOn();

	// Tasks and events
	static void startRXTask();
	static void startTXTask();
#ifdef USE_PACKET_DONE_FOR_EOT
	static bool isPacketDone();
	static void clearPacketDoneEvent();
#else
	// Not implemented or used: static bool isEOTEvent();
	static void clearMsgReceivedEvent();
	static void clearEndTransmitEvent();
#endif

	/*
	 * Events are NOT SAME as state.
	 * !!! DISABLE task sets event and state
	 * DISABLE event and sate set by either DISABLE Task OR after packet done
	 */
	static void startDisablingTask();
	// events
	static bool isDisabledEventSet();
	static void clearDisabledEvent();
	// state remains in effect even after even is cleared
	static bool isDisabledState();

	static bool isReceiveInProgressEvent();
	static void clearReceiveInProgressEvent();

#ifdef USE_PACKET_DONE_FOR_EOT
	static void enableInterruptForPacketDoneEvent();
	static void disableInterruptForPacketDoneEvent();
	static bool isEnabledInterruptForPacketDoneEvent();
#else
	// use disabled event for EOT
	static void enableInterruptForDisabledEvent();
	static void disableInterruptForDisabledEvent();
	static bool isEnabledInterruptForDisabledEvent();
#endif

	static void setShortcutsAvoidSomeEvents();

	/*
	 * Result is for most recently received packet.
	 * If called before any receive, result is meaningless.
	 */
	static unsigned int receivedSignalStrength();
};
