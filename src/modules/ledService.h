
#pragma once

#include "../drivers/gpioDriver.h"



typedef unsigned int LEDOrdinal;

/*
 * Manage array of LED's.
 *
 * This abstracts from an ordinal to a GPIO index.
 * I.E. you pass GPIO pins on init(), and thereafter an LED ordinal.
 * It also abstracts lit/dark from pin state (1/0) and electrical levels (source or sunk.)
 *
 * Safe to log to any ordinal, no effect if ordinal is not mapped to a GPIO index at config time.
 *
 * Safe to have more than one instance.
 * E.G. a library can have its own instance,
 * but of course you must coordinate use of LEDs.
 *
 * Toggle: LED's stay lit/dark until the next call,
 * so they indicate state of debugging by their transitions.
 *
 * !!! This is not thread safe: it doesn't test that pins remain configured as outputs.
 * I assume that writing to a pin that is configured as input is harmless.
 *
 * You must call init() to guarantee pins are configured properly:
 * Valid sequence is: init(), toggleLEDs(), ...
 */

class LEDService {
public:
	static void init(unsigned int count, bool arePinsSunk, GPIOIndex led1GPIO, GPIOIndex led2GPI0, GPIOIndex led3GPIO, GPIOIndex led4GPIO);
	static void toggleLEDsInOrder();
	static void toggleLED(LEDOrdinal ordinal);
	static void switchLED(LEDOrdinal, bool state);
};
