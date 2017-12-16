#pragma once

/*
 * nested vectored interrupt controller
 *
 * !!! Nvic is the class, NVIC is a Nordic macro
 */
class Nvic {
public:
	// pair of methods for each IRQ

	static void enableRadioIRQ();
	static void disableRadioIRQ();

	static void enableRTC1IRQ();
	// Often never called, since RTC runs forever in many apps
	static void disableRTC1IRQ();
	static void pendRTC1Interrupt();


	static void enablePowerClockIRQ();
	static void disablePowerClockIRQ();

	//static bool isEnabledPowerClockIRQ();
	// FUTURE static bool isEnabledRadioIRQ();

	// Trigger reset via software
	static void softResetSystem();


};
