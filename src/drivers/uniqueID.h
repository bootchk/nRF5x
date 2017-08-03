
#pragma once

#include <inttypes.h>


/*
 * A substring (6 bytes of) full MAC ID (8 bytes) of radio as provided by platform.
 * Embedded in payload.
 * Not the same as protocol address.
 */
uint64_t deviceID();


