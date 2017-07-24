
#include <cassert>

#include "mailbox.h"


/*
 * This implementation:
 * - holds only one item.
 * - is not generic on type of object held
 * - is not thread safe
 *
 * - is not static, i.e. this data members
 */



void Mailbox::put(WorkPayload aItem){
	// FUTURE Thread safe: atomic
	assert(! isItem);
	item = aItem;
	isItem = true;
}

WorkPayload Mailbox::fetch(){
	assert(isItem);
	// Thread safe: copy item before deleting from queue
	WorkPayload result = item;
	isItem = false;
	return result;
}

bool Mailbox::isMail(){
	return isItem;
}
