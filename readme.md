
Sparse, object oriented (C++) library for nRF5x family embedded radios.


Provides
-
      
     Thin wrappers around certain RTC peripherals of the Nordic nRF5x family of embedded radio SoC chips.
     Other simple services such as a mailbox
     Thin wrapper around certain board-level devices such as LED's.
     
     
One build configuration defines BOARDS\_CUSTOM which means included file boards.h will include custom_board.h from this directory.  Another build configuration builds for the nRF52DK board.


Motivation
-
Nordic's libraries suspect and overweight for my use case.

    
Implementation Notes
-
Some use of HAL, not nrf_drv: probably less robust, but easier to understand.  

Some bare metal, direct access to device registers.


Building
-

The artifacts are libraries cross-compiled for targets nRF51 and nRF52 (ARM M0 and M4.)

The project in this repository is an Eclipse project, using Eclipse's build system.
The project's Eclipse build configuration contains many include paths and other build settings.
Such settings are not defined in a Makefile.

See Also
-
 Derived from (and superseding) nRFCounter.

A using project will be fireFly.


