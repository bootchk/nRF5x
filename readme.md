
Sparse, object oriented (C++) library for nRF5x family embedded radios.


Provides
-
      
     Thin wrappers around certain RTC peripherals of the Nordic nRF5x family of embedded radio SoC chips.
     Other simple services such as a mailbox
     Thin wrapper around certain board-level devices such as LED's.

     



Motivation
-
Nordic's libraries suspect and overweight for my use case.

    
Implementation Notes
-
Some use of HAL, not nrf_drv: probably less robust, but easier to understand.  

Some bare metal, direct access to device registers.


Building
-

Several build configurations:


    artifacts are libraries cross-compiled for targets nRF51 and nRF52 (ARM M0 and M4.)
    artifacts are executables that test the library

The project in this repository is an Eclipse project, using Eclipse's build system.
The project's Eclipse build configuration contains many include paths and other build settings.
Such settings are not defined in a Makefile.

One important linker setting is --specs=nosys.specs otherwise get undefined reference to _exit.

Boards
-
Some services (e.g. ledLogger) depend on the board configuration (what pins are configured as digital out to LEDs, and what revision of the chip is on the board.)

One build configuration for the nRF51 family defines BOARDS\_CUSTOM which means included Nordic file boards.h will include custom_board.h for RedBear Nano (one LED) from this directory.  

Another build configuration for nRF52 family builds for the nRF52DK board (four LED's.)  Defines BOARD_pca10040.

Debugging
-

Eclipse with the GNU ARM Plugin supports the nRF52DK and its Segger debug probe nicely.  Plug in your DK dev kit's USB and choose *Debug As>Debug Configurations>nRF5x DebugNRF52*.

(You may want to create your own Debug Configuration.)

To see the log, open a terminal and run JLinkRTTClient, which you can download from Segger.

Testing nRFCounter, you should see a sequence of times scroll by.

TODO add other mains that test other modules.

See Also
-
 Derived from (and superseding) nRFCounter and nRFrawProtocol
 
 See testnRFCounter project, a template project with similar build and debug configurations.

A using project will be fireFly.

Environment
-
I use:

     nRF52DK development board having SEGGER debug probe
     RedBear Nano having nRF51 (debugged via SWC cables to nRF52DK)
     Ubuntu 16.04 LTS
     Eclipse Neon version
     Eclipse ARM GCC plugin
     Nordic NRF_SDK v12

