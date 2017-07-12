
#include <cassert>


#include "ensemble.h"

#include "../radio/radio.h"
#include "../drivers/hfClock.h"
#include "../drivers/powerSupply.h"
#include "../clock/clockFacilitator.h"

// temp, for measuring varying startup duration
#include "../clock/longClock.h"
#include "../services/logger.h"


void Ensemble::init(MsgReceivedCallback aCallback) {

	assert(! HfCrystalClock::isRunning());	// xtal not running

	// On some platforms, it stays configured.
	Radio::configure();

	Radio::setMsgReceivedCallback(aCallback);
}

// The only member that needs configuration is Radio
bool Ensemble::isConfigured(){ return Radio::isConfigured(); }


bool Ensemble::isLowPower() {
#ifdef RADIO_POWER_IS_REAL
	return ((!Radio::isPowerOn()) && !Radio::hfCrystalClock->isRunning());
#else
	// Radio will be low power if not in use
	return ((!isRadioInUse()) && !HfCrystalClock::isRunning());
#endif
}

/*
 * Methods simply to the Radio.
 */
bool Ensemble::isRadioInUse() 			   { return Radio::isInUse(); }
BufferPointer Ensemble::getBufferAddress() { return Radio::getBufferAddress(); }
bool Ensemble::isPacketCRCValid()          { return Radio::isPacketCRCValid(); }



/*
 * Startup must be called before StartReceiving or Transmit
 */
void Ensemble::startup() {

	// enable this first so it has time to ramp up
	// assert enough power for radio => >2.1V required by DCDCPowerSupply
	DCDCPowerSupply::enable();

#ifdef RADIO_POWER_IS_REAL
	Radio::powerOn();
	// Some platforms require config after powerOn()
	Radio::configurePhysicalProtocol();
#else
	// Radio stays powered and configured.
#endif

	/*
	 * Timing varies by board, since different crystal models used.
	 * NRF52DK: 11 ticks == 360uSec
	 * Waveshare nRF51: 40 ticks == 1200uSec
	 */
	// temp
	LongTime startTime = LongClock::nowTime();
	/*
	 * OLD
	 * Formerly, startup required varying duration.
	 * Now, startup takes a constant duration.
	 *
	 * HfCrystalClock::startAndSleepUntilRunning();
	 */
	// TODO symbolic constant that depends on the board
	ClockFacilitator::startHFClockWithSleepConstantExpectedDelay(40);
	logInt(LongClock::nowTime() - startTime);
	log("<hfclock start\n");

	assert(Radio::isConfigured());
}

void Ensemble::shutdown() {
	HfCrystalClock::stop();

#ifdef RADIO_POWER_IS_REAL
	Radio::powerOff();
#else
	// If not in use, will enter standby low-power mode automatically
	assert(! Radio::isInUse());
#endif

	// disable because Vcc may be below what DCDCPowerSupply requires
	DCDCPowerSupply::disable();
}



void Ensemble::startReceiving() {
	/*
	 * Not log to flash here, since it takes too long.
	 */

	// TODO should this be in caller?
	// OLD syncSleeper.clearReasonForWake();

	assert(Radio::isPowerOn());
	Radio::receiveStatic();
	assert(Radio::isInUse());

	/*
	 * SyncSleeper will clearReasonForWake().
	 * Thus there is a low probablity race here.
	 * Any message that arrives before SyncSleeper clears reason might be lost.
	 * But it is low probability since there is a rampup time (40-140 uSec, i.e. 700-2100 instruction cycles) for the radio
	 * to go from disabled to active.
	 * We almost always will sleep before the radio is able to receive.
	 */
}



void Ensemble::stopReceiving() {
	if (Radio::isInUse()) {
		Radio::stopReceive();
	}
	assert(!Radio::isInUse());
}


void Ensemble::transmitStaticSynchronously(){
	assert(Radio::isPowerOn());
	// also must be configured

	Radio::transmitStaticSynchronously();
}

