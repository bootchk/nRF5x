
/*
 * Generic fault handlers.
 *
 * This is specialized for this ultra-low power app:
 * - in production, soft reset
 * - in debugging, turn off devices (radio and RTC) and sleep in a loop.
 *
 * Your app must redefine the default handlers to call these generic handlers.
 * The default handlers are weakly defined and 'called' from the interrupt vectors
 * (Nordic: in gcc_startup.S)
 * In general, the default handlers are inadequate, simply looping.
 * Looping will lead high current consumption, to brownout, then power recovery, and reset.
 * In that scenario, the app may appear to keep functioning, but have gone through a reset.
 *
 * These are generic and should not depend on the target i.e. Nordic.
 * I.E. the implementation should call other modules, not Nordic dependent drivers.
 */

extern "C" {

void genericNMIHandler();    // Certain peripherals or SW. Non-maskable, only preempted by reset
void genericSVCHandler();    // call to OS: SVC instruction executed
void genericPendSVHandler(); // OS Context switching
void genericSysTickHandler(); // OS clock

// M0 bus faults and other hw faults
// M4 also defines other faults, subclasses, having their own vectors
void genericHardFaultHandler();

/*
 * Handler for:
 * - NRF_SDK library errors.
 * - assert() macro (Nordic definition of assert() calls this?)
 *
 * The default handler does: app_error_save_and_stop(id, pc, info);
 */
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info);


}	// extern C
