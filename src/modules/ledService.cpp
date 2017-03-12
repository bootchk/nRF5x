/*
 * Implementation notes:
 *
 * This does NOT depend on board configuration e.g. nrf_sdk/examples/bsp/boards.h
 * The caller might use that to know a board's use of GPIO pins e.g. LEDS_LIST and LEDS_NUMBER.
 * But this library is agnostic of board.
 *
 * Nordic's boards.h in turn includes e.g. custom_board.h or examples/bsp/pca10040.h,
 * as determined by a preprocessor definition -D BOARD_CUSTOM or -D BOARD_PCA10040
 * Those include files define the macros: LEDS_LIST, LEDS_NUMBER, etc
 * Importantly, they define LED_INV_MASK which determines whether LED's are sinked or sourced.
 * Thus determining whether 'lit' requires high/low state on a GPIO.
 */

#include <modules/ledService.h>
#include <cassert>
#include "../drivers/gpioDriver.h"


namespace {

const int MaxLedCount = 4;

/*
 * Map from ordinal to GPIO pin in range [0..31]
 *
 * Fixed size, only a prefix might be used.
 */
GPIOIndex ledOrdinalToPinMap[MaxLedCount];
unsigned int ledCount;
GPIOMask allLedPinsMask;

GPIODriver gpio;


void createMap(int count, GPIOIndex led1GPIO, GPIOIndex led2GPIO, GPIOIndex led3GPIO, GPIOIndex led4GPIO){
	assert(count<=MaxLedCount);
	ledOrdinalToPinMap[0] = led1GPIO;
	if (count > 1)
		ledOrdinalToPinMap[1] = led2GPIO;
	if (count > 2)
		ledOrdinalToPinMap[2] = led3GPIO;
	if (count > 3)
		ledOrdinalToPinMap[3] = led4GPIO;
	// map is undefined beyond count
}

GPIOMask createMaskOfManagedIndices(unsigned int count, GPIOIndex led1GPIO, GPIOIndex led2GPIO, GPIOIndex led3GPIO, GPIOIndex led4GPIO){
	assert(count<=MaxLedCount);
	GPIOMask result = 0;
	result = 1 << led1GPIO;
	if (count > 1)
		result |= 1 << led2GPIO;
	if (count > 2)
		result |= 1 << led3GPIO;
	if (count > 3)
		result |= 1 << led4GPIO;
	return result;
}

GPIOMask maskFromOrdinal(unsigned int ordinal) {
	assert (! ((ordinal < 1) || (ordinal > ledCount)));

	GPIOMask result = 0;
	result = 1 << ledOrdinalToPinMap[ordinal-1];

	assert(result != 0);
	return result;
}

}	// namespace





void LEDService::init(unsigned int count, bool arePinsSunk, GPIOIndex led1GPIO, GPIOIndex led2GPI0, GPIOIndex led3GPIO, GPIOIndex led4GPIO){
	/*
	 * configure GPIO pins as digital out to LED.
	 * This references constants defined by macros, not ledOrdinalToPinMap.
	 */

	assert(count<5 and count>0);
	// Similar to Nordic board.h LED macros, but at runtime, not at macro time
	ledCount = count;
	createMap(count, led1GPIO, led2GPI0, led3GPIO, led4GPIO);
	allLedPinsMask = createMaskOfManagedIndices(count, led1GPIO, led2GPI0, led3GPIO, led4GPIO);

	gpio.init(allLedPinsMask, arePinsSunk);
	// !!! off before enable out so no glitch
	gpio.turnOff(allLedPinsMask);
	gpio.enableOut(allLedPinsMask);

	// assert LED GPIO pins configured
	// assert self count, map, allLedsMask are initialized
	// ensure LEDs are dark
	assert(! gpio.isOn(allLedPinsMask));
}

void LEDService::toggleLEDsInOrder() {
	for (unsigned int i = 0; i < ledCount; i++)
	{
		toggleLED(i+1);	// index to ordinal
	}
}

void LEDService::toggleLED(unsigned int ordinal) {
	gpio.invert(maskFromOrdinal(ordinal));
}

void LEDService::switchLED(unsigned int ordinal, bool state) {
	if (state)
		gpio.turnOn(maskFromOrdinal(ordinal));
	else
		gpio.turnOff(maskFromOrdinal(ordinal));
}

