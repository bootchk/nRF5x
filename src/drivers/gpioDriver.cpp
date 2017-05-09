/*
 * Implementation notes:
 *
 * Mostly taken from boards.h
 *
 * Obscure: logical operations to create a mask that embodies sunk/sourced.
 */

#include <cassert>

#include <nrf.h>
#include <nrf_gpio.h>
#include "gpioDriver.h"

// ??? not sure why I use a mix of raw registers and nrf_gpio methods


namespace {

// Mask of all pins managed, captured at config time
GPIOMask managedPinsMask;

// Mask of sunk pins: logic low pulls current from LED in series with Vcc.
GPIOMask sunkPinsMask;

/*
 * Not sure this is necessary:
 * some forum posts say that the reset condition guarantees low power
 * while others say that NOPULL should nt be used.
 */
void initLowPower() {
	// Set every pin to IN, DISCONNECT,
	for (uint32_t pin = 0; pin < 32; pin++) {

		// Configure high current output (max 5mA)
		nrf_gpio_cfg(
				pin,
				NRF_GPIO_PIN_DIR_INPUT,
				NRF_GPIO_PIN_INPUT_DISCONNECT,
				NRF_GPIO_PIN_PULLDOWN,
				NRF_GPIO_PIN_H0H1,	// !!! high current
				NRF_GPIO_PIN_NOSENSE);
	}
}


}	// namespace


void GPIODriver::init(GPIOMask mask, McuSinksOrSources arePinsSunk) {
	// Does not allow a mix of sunk and source managed pins
	managedPinsMask = mask;
	if (arePinsSunk == McuSinks)
		sunkPinsMask = mask;
	else
		sunkPinsMask = ~mask;	// sunkPinsMask has bits set where not set in managedPinsMask

	// TODO test whether this is necessary
	initLowPower();
}

void GPIODriver::enableOut(GPIOMask mask) {
	assert(mask & managedPinsMask);

	// Iterate over bits of mask: separate config register for each pin
	uint32_t pin;
	for (pin = 0; pin < 32; pin++)
		if ( mask & (1 << pin) ) {
			// Convenience function for standard drive current (max 0.5mA)
			// nrf_gpio_cfg_output(pin);

			// Configure high current output (max 5mA)
			nrf_gpio_cfg(
					pin,
					NRF_GPIO_PIN_DIR_OUTPUT,
					NRF_GPIO_PIN_INPUT_DISCONNECT,
					NRF_GPIO_PIN_NOPULL,
					NRF_GPIO_PIN_H0H1,	// !!! high current
					NRF_GPIO_PIN_NOSENSE);
		}
}

/*
 * On and Off are logical states.
 * The physical pin may be electrical low to effect logical on when pin is sinking.
 */
void GPIODriver::turnOn(GPIOMask mask) {
	assert(mask & managedPinsMask);
	assert( ! (mask & ~managedPinsMask));

	NRF_GPIO->OUTCLR = mask & (managedPinsMask & sunkPinsMask);
	NRF_GPIO->OUTSET = mask & (managedPinsMask & ~sunkPinsMask);
}

void GPIODriver::turnOff(GPIOMask mask){
	assert(mask & managedPinsMask);  // mask is subset of managed pins
	assert( ! (mask & ~managedPinsMask));

	NRF_GPIO->OUTSET = mask & (managedPinsMask & sunkPinsMask);
	NRF_GPIO->OUTCLR = mask & (managedPinsMask & ~sunkPinsMask);
}

void GPIODriver::invert(GPIOMask mask){
	// assert mask is subset of managed pins
	// Here we don't care whether sunk: just invert it
	GPIOMask gpio_state = NRF_GPIO->OUT;
	NRF_GPIO->OUTSET = mask & ~gpio_state;
	NRF_GPIO->OUTCLR = mask & gpio_state;
}

bool GPIODriver::isOn(GPIOMask mask){
	// Exclusive OR state with sunkPinsMask
	// state mask result
	// 1      0    1     high, sourced, on/lit
	// 1      1    0     high, sunk,    off/dark
	// 0      0    0     low,  sourced, off/dark
	// 0      1    1     low,  sunk,    on/lit
	return mask & (NRF_GPIO->OUT ^ sunkPinsMask);
}
