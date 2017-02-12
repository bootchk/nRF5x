/*
 * Driver for GPIO device.
 *
 * Manages a subset of GPIO pins.
 * Does not affect pins not managed.
 * All pins are configured the same way (out, and sink/source the same for each pin.)
 *
 * Algebra of legal calling order:
 * configureOut(), turnOn() etc
 * I.E. configureOut must be called first with the full set of managed pins.
 * Subsequent calls pass a mask of a subset of pins.
 */

#pragma once

#include <inttypes.h>

// GPIO index is not the same as a chip's pin number
typedef uint8_t GPIOIndex;	// [0..31]

typedef uint32_t GPIOMask;



class GPIODriver {
public:
	static void configureOut(GPIOMask allPinsMask, bool arePinsSunk);
	/*
	 * On and Off are logical states.
	 * The physical pin may be electrical low to effect logical on when pin is sinking.
	 */
	static void turnOn(GPIOMask mask);
	static void turnOff(GPIOMask mask);
	static void invert(GPIOMask mask);
	static bool isOn(GPIOMask mask);
};
