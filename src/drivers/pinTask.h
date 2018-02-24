
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
public:
	static void configureTaskOnPinToToggleInitiallyHigh( uint32_t pin );
	static void enableTask();
	static void disableTaskAndForceToInitialState();

	// Needed to hook to PPI
	static uint32_t getTaskRegisterAddress();
};
