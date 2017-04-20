/*
 * Driver for GPIO device.
 *
 * Manages a subset of GPIO pins.
 * Does not affect pins not managed.
 * All pins are configured the same way (out, and sink/source the same for each pin.)
 *
 * Algebra of legal calling order:
 * init(), turnOff(), enableOut(), ...
 * init(), turnOn(), enableOut(), ...
 * I.E. init() must be called first with the full set of managed pins.
 * Subsequent calls pass a mask of a subset of pins.
 * To avoid a glitch, turnOff or turnOn before enableOut()
 *
 * State of GPIO and electrical state of hw pin persists during sleep.
 */

#pragma once

#include <inttypes.h>

// GPIO index is not the same as a chip's pin number
typedef uint8_t GPIOIndex;	// [0..31]

typedef uint32_t GPIOMask;

enum McuSinksOrSources {
	McuSources =0,
	McuSinks
};



class GPIODriver {
public:
	/*
	 * Configure driver: has no effect on hw pins.
	 */
	static void init(GPIOMask allPinsMask, McuSinksOrSources arePinsSunk);

	/*
	 * Set GPIO direction to OUT.
	 * This enables, i.e. connects GPIO state to external pin.
	 * Before: GPIO pin direction could be IN, OUT, or disconnected.
	 * After: GPIO pin direction is OUT and the electrical level on the pin corresponds to state.
	 *
	 * !!! If you know what state you want, set it before you enableOut().
	 * Otherwise, you might get a glitch between enableOut() and setting state e.g. turnOff.
	 */
	static void enableOut(GPIOMask allPinsMask);
	/*
	 * On and Off are logical states.
	 * The physical pin may be electrical low to effect logical on when pin is sinking.
	 */
	static void turnOn(GPIOMask mask);
	static void turnOff(GPIOMask mask);
	static void invert(GPIOMask mask);
	static bool isOn(GPIOMask mask);
};
