
#pragma once

/*
 * Hides implementation.
 * Two implementations: SD compatible and not.
 *
 * NVIC nested vectored interrupt controller.
 * NVIC is not a device, but inside the mpu.
 * The SD requires a certain API to control NVIC so that SD can know and check your usage.
 * Since NVIC is in mpu, SD cannot use its usual mechanism to protect certain memory locations,
 * but requires you to funnel your calls through its sd_nvic_... API.
 *
 * !!! Nvic is the class, NVIC is a Nordic macro
 */
class NvicCoordinated {
public:
	// pair of methods for each IRQ

	static void enableRadioIRQ();
	static void disableRadioIRQ();

	/*
	 * LFTimer is one of the RTCx where x in [0,2] on NRF52
	 */
	static void enableLFTimerIRQ();
	// Often never called, since RTC runs forever in many apps
	static void disableLFTimerIRQ();
	static void pendLFTimerInterrupt();


	static void enablePowerClockIRQ();
	static void disablePowerClockIRQ();

	// FUTURE
	//static bool isEnabledPowerClockIRQ();
	// static bool isEnabledRadioIRQ();

	// Trigger reset via software
	static void softResetSystem();


};
