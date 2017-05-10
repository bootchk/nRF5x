
/*
 * Functions that assert() that certain kinds of reset have not happened.
 *
 * Used in debugging.
 */

// Raises assertion if any reset other than POR or BOR have occurred since POR or call to clearResetReason
void assertNoResetsOccurred();
void clearResetReason();


