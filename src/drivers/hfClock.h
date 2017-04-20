#pragma once

#include "nvic.h"

/*
 * The HF (16Mhz) xtal clock.
 * Required by radio.
 *
 * See a note on Nordic forums: https://devzone.nordicsemi.com/question/18096/what-is-the-high-frequency-clock-model-for-nrf51/
 * Briefly it says a HF RC (HFINT) clock is used by other parts of the system when the HF xtal clock is stopped.
 * And the HF RC clock starts and stops automatically when the HF xtal clock is stopped.
 * The radio needs a running HF xtal clock, but a wireless stack may stop it to save power.
 *
 * It takes upwards of 0.5 mSec to start and become stable.
 * (400uSec nrf51, 360 uSec nrf52, max)
 *
 * The event HFCLKSTARTED actually means "running and stable" and does not mean "start task has been triggered."
 *
 * See comments in hfClock.c.
 *
 * This is only the HF clock whose source is xtal.
 * The other HF clock is not really controllable by the app.
 */

class HfCrystalClock {

private:
	static void start();


public:
	// Uses nvic
	static void init(Nvic* nvic);

	static bool isRunning();

	static void stop();

	/*
	 * Spin mcu while starting.
	 */
	static void startAndWaitUntilRunning();

	/*
	 * Sleep during startup to avoid energy consumption.
	 * Does not return until running, but is low power until then.
	 * HfCrystalClock takes about 0.5 mSec to run stable.
	 */
	static void startAndSleepUntilRunning();
};
