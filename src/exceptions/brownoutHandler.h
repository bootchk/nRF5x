
#pragma once

#include <inttypes.h>

/*
 * Write faultAddress to flash if not already written.
 *
 * Called in a brownout state (EVENTS_POFWARN is set)
 */
void brownoutWritePCToFlash(uint32_t faultAddress);
