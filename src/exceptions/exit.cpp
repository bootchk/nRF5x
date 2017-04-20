

/*
 * Override the function defined in newlib nano.
 * To catch soft exceptions.
 *
 * assert calls abort() and then _exit()
 *
 * The _exit() defined in newlib nano infinite loops.
 */

/*
 // TODO attribute noreturn
  // TODO WFI as a halt or an infinite loop
extern "C"
void _exit() {

	int i = 1;

	i++;
}
*/
