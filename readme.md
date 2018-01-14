
Sparse, object oriented (C++) driver library for nRF5x family embedded radio chips.

Provides
-
     Wrappers for certain Nordic nRF5x family of embedded radio SoC chips.
     Drivers for the mcu, nvic, flash memory
     Drivers for certain commonly used peripherals (clocks, counters, power supplies, radio, etc.)

Motivation
-
    Nordic's libraries overweight for my use case.
    I wanted a lightweight radio protocol (not Bluetooth)
    I didn't think I needed an RTOS, but needed some basics e.g. timers and mailbox

See History below.  Much was separated into a platform independent library radioSoC.
    
    
Implementation Notes
-

HW resources can be used via :

    bare metal, direct access to device registers
    Nordic's HAL (abstraction across various Nordic chips)
    Nordic's drivers  nrf_drv_<foo> in NRF\_SDK/components/nrf_drv
   
HAL
 
Library mostly uses HAL but also some bare metal.  (Probably HAL should be used exclusively so it could be ported to another chip?)

HAL:  implements in-line (see the Nordic .h files.)  Thus few extra Nordic source files (.c) are needed in the project or to use the library.


Nordic drivers

Nordic drivers are required when Softdevice is also used (to coordinate access to shared resources.)
Softdevice can be used for sequential multiprotocol to use BT for provisioning.



Building
-

Several build configurations:

    artifacts are libraries cross-compiled for targets nRF51 and nRF52 (ARM M0 and M4.)
    artifacts are executables that test the library

The project in this repository is an Eclipse project, using Eclipse's build system.
The project's Eclipse build configuration contains many include paths and other build settings.
Such settings are not defined in a Makefile.

The most important setting is *Properties>C/C++ Build>Build Variables* variable named NRF_SDK should have a value pointing to your installation of the Nordic SDK.

Other important settings:

    linker setting --specs=nosys.specs otherwise get undefined reference to _exit.
    Tool Settings>Target Processor  float abi hard (nRF52 has FPU)
    *Properties>C/C++ General>Paths and Symbols>Symbols* define NRF52 or NRF51, and a board e.g. PC10040?
    


AFAIK, many of the other defines for PAN's (Product Anomalies) found in Nordic Makefiles are not needed for this library.  They seem to affect code conditionally compiled into certain Nordic libraries.

Build configurations
-

NRF51 build configuration uses NRF_SDK v12.3 (last version to support 51)

NRF52 Now using v14.2 
Early version was not Softdevice compatible.  Now is Softdevice compatible.


Debugging
-

Eclipse with the GNU ARM Plugin supports the nRF52DK and its Segger debug probe nicely.  Plug in your DK dev kit's USB and choose *Debug As>Debug Configurations>nRF5x DebugNRF52*.

(You may want to create your own Debug Configuration.)

To see the log, open a terminal and run JLinkRTTClient, which you can download from Segger.

Testing nRFCounter, you should see a sequence of times scroll by.

TODO add other mains that test other modules.


Boards, Logging, Exception Handlers
-

Board configuration, logging, exception handlers not provided.

Instead, they are provided in higher layers.


History
-
Derived from (and superseding) nRFCounter and nRFrawProtocol
 
See testnRFCounter project, a template project with similar build and debug configurations.  More explanation there about exactly how to configure a Build Configuration in Eclipse.

Early versions were huge.  Since split into this project of only drivers of Nordic hardware, and lib radioSoC, intended to be platform inpependent.

A using project is fireFly.


Environment
-
I use:

     nRF52DK development board having SEGGER debug probe
     RedBear Nano having nRF51 (debugged via SWC cables to nRF52DK)
     Ubuntu 16.04 LTS
     Eclipse Neon version
     Eclipse ARM GCC plugin
     Nordic NRF_SDK v12

