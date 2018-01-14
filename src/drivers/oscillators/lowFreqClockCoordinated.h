#pragma once

/*
 * Low frequency oscillator (not really a clock.)
 *
 * Coordinated: allows multiprotocol, i.e. compatible with SD.
 * Allows multiple users of the clock, keeps clock running until last user finished with clock.
 * Some protocols never stop the clock, but for generality, the SD assumes it might be stopped.
 *
 * Configured by sdk_config.h when the underlying "clock module" nrf_drv_clock.c is compiled.
 * Main configurations is source: xtal or rc.

 *
 * The states are: !started, started, !running, running
 * started does not guarantee running!!!
 * !!! These are the states of this API.
 * The states of the hardware might be different and confusing.
 *
 * Valid call sequences:
 *
 * The normal call sequence:
 * init, start, isStarted returns true, while(!isRunning()) {}, isRunning returns true
 *
 * You must busy wait for isRunning:
 * isStarted returns false, start, isStarted returns true, isRunning returns false
 */

// XXX methods const (no data members to write)


typedef void (*Callback)();



class LowFreqClockCoordinated {
public:
	/*
	 * Coordination requires init.
	 */
	static void init();

	/*
	 * Callbacks from ISR with interrupts disabled, should be short.
	 */
	static void registerCallbacks(Callback, Callback);


	static void start();


	/*
	 * Was previous call to start()?
	 * Does not guarantee isRunning() or not stopped
	 */
	static bool isStarted();

	/*
	 * Is clock running (and stable)?
	 */
	static bool isRunning();

	/*
	 * !!! No stop() defined.
	 * (Not needed by most apps.)
	 * Assume the PowerManager functions as designed and does not power down clock.
	 */

	/*
	 * Waste cpu cycles until running.
	 * Usually not used.
	 * A more power efficient design is to wait for an interrupt from completion of calibration
	 * OR wait for an interrupt from counter whose source is the oscillator.
	 */
	static void spinUntilRunning();
};
