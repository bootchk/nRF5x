#pragma once

/*
 * Low frequence oscillator (not really a clock.)
 *
 * The states are: !started, started, !running, running
 * started does not guarantee running!!!
 *
 * Valid call sequences:
 *
 * The normal call sequence:
 * configureXtalSource, start, isStarted returns true, while(!isRunning()) {}, isRunning returns true
 *
 * start, isStarted, ... (uses default LFRC oscillator source)
 *
 * You must busy wait for isRunning:
 * isStarted returns false, start, isStarted returns true, isRunning returns false
 *
 * You must configure before starting:
 * start, configureXtalSource will halt
 *
 *
 */

// XXX methods const (no data members to write)

class LowFrequencyClock {
public:

	static void clockISR();	// Also for HFClock

	static void enableInterruptOnStarted();
	static void disableInterruptOnStarted();
	static void configureXtalSource();

	static void start();


	/*
	 * Was there a prior call to start() AND did the LFCKLSTARTED event occur?
	 * !!! Does not guarantee isRunning()
	 */
	static bool isStartedEvent();


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
