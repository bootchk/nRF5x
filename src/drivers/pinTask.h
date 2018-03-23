
#pragma once

#include <inttypes.h>


/*
 * Facade for gpiote device.
 *
 * Implements task aspect only (not events from pins.)
 *
 * Configure task registers that drive a pin.
 * Where a task register can be driven by the PPI.
 * Thus some system event (say a timer) triggers a task in the gpiote which changes a pin state,
 * all without cpu intervention.
 */

// Non-parameterized implementation: one channel, toggle, initially high
class PinTask {
private:
	// Not used
	static void disableTaskAndForceToInitialState();

public:
	/*
	 * Configure tasks for an out, sunk pin.
	 *
	 * Tasks are SunkOn and SunkOff.
	 * Pin is a sink, initially high (not conducting)
	 */
	static void configureSunkPinTasks( uint32_t pin );

	/*
	 * Usually only enabled once, and never disabled.
	 */
	static void enableTask();

	/*
	 * Needed to hook to PPI.
	 * We are only using sunkOff w PPI
	 */
	static uint32_t* getSunkOffTaskRegisterAddress();

	static void startSunkOffTask();
	static void startSunkOnTask();

	/*
	 * !!! While GPIOTE used, GPIO.OUT does NOT show state of pin.
	 * !!! Obscure:  GPIO.IN does show state of pin.
	 */
	// static bool pinState();


};
