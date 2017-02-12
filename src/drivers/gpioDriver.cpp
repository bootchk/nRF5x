/*
 * Implementation notes:
 *
 * Mostly taken from boards.h
 *
 * Obscure: logical operations to create a mask that embodies sunk/sourced.
 */
#include <nrf.h>
#include <nrf_gpio.h>
#include "gpioDriver.h"

// ??? not sure why I use a mix of raw registers and nrf_gpio methods


namespace {

// Mask of all pins managed, captured at config time
GPIOMask managedPinsMask;

// Mask of sunk pins: logic low pulls current from LED in series with Vcc.
GPIOMask sunkPinsMask;
}




void GPIODriver::configureOut(GPIOMask mask, bool arePinsSunk) {
	// Capture.
	// Does not allow a mix of sunk and source managed pins
	managedPinsMask = mask;
	if (arePinsSunk)
		sunkPinsMask = mask;
	else
		sunkPinsMask = ~mask;	// sunkPinsMask has bits set where not set in managedPinsMask


	// Configure pin direction out
	uint32_t pin;
	for (pin = 0; pin < 32; pin++)
		if ( mask & (1 << pin) )
			nrf_gpio_cfg_output(pin);

}
/*
 * On and Off are logical states.
 * The physical pin may be electrical low to effect logical on when pin is sinking.
 */
void GPIODriver::turnOn(GPIOMask mask) {
	NRF_GPIO->OUTCLR = mask & (managedPinsMask & sunkPinsMask);
	NRF_GPIO->OUTSET = mask & (managedPinsMask & ~sunkPinsMask);
}

void GPIODriver::turnOff(GPIOMask mask){
	// assert mask is subset of managed pins
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
