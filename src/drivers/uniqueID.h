
#pragma once

#include <inttypes.h>

/*
 * Knows properties of the system
 * - deviceID
 * - FUTURE versions of software
 */
class SystemProperties {
public:
	/*
	 * A substring (6 bytes of) full MAC ID (8 bytes) of radio as provided by platform.
	 * Protocol stacks embed it in messages.
	 * Not the same as protocol address.
     * unique ID burned at factory
	 */
	static uint64_t deviceID();
};
