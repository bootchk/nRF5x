#pragma once

/*
 * "driver" for microcontroller device.
 * Hides target specifics.
 *
 * AKA system
 *
 *
 */
class MCU {
public:
	/* sleep mcu until an event */
	static void sleep();
};
