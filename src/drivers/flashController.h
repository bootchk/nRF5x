#pragma once

#include <inttypes.h>


/*
 * non volatile memory controller
 */
class FlashController {

public:
	static const uint32_t UICRStartAddress = 0x10001080;	// target dependent

	static bool isDisabled();

	// !!! Both have busy waits
	static void enableWrite();
	static void disableWrite();
};
