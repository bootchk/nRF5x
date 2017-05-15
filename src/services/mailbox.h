
#pragma once

#include "../types.h"	// library types

/*
 * Simple mailbox:
 * - holding one item
 * - listener polls
 * - not thread-safe (only one poster and listener)
 *
 * Statically configured to empty.
 * Algebra:
 * reset; isMail() == false
 * put(); isMail() == true; fetch(); isMail() == false
 *
 * Note there is no init() to empty the mailbox.
 * After an exception, it might be necessary to flush the mailbox by reading it.
 */



class Mailbox {
	WorkPayload item;
	bool isItem = false;

public:
	// put overwrites a mail when overflows
	void put(WorkPayload item);

	// fetch first mail in box (if queued)
	WorkPayload fetch();

	bool isMail();
};
