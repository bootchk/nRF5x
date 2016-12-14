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

	static void enableRTC0IRQ();
	// Often never called, since RTC runs forever in many apps
	static void disableRTC0IRQ();

	// FUTURE static bool isEnabledRadioIRQ();
};
