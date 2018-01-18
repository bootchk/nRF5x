
nRF5x library

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

All these configs are debugging: assertions enabled and no optimization.

Debug51    NRF51 . NRF_SDK v12.3 (last version to support 51)

Debug52    NRF52 Now using v14.2. Not Softdevice compatible.  Uses HAL which is in-lined from Nordic .h files.

Debug52SD    nrf52 SDK14.2  Softdevice (and whatever nrf_log() is enabled in sdk_config.h

test52     NRF52  Build a main that minimally calls the library (often out of date.)



Multiprotocol
-

SOFTDEVICE_PRESENT	Whether library is compatible with Softdevice.  Same symbol as used in NRF_SDK

SOFTDEVICE_PRESENT usually means "sequential multiprotocol" i.e. alternate between using the Softdevice for Bluetooth protocol and implementing your own protocol.

Using Softdevice means the implementation is restricted: can't define some ISR's and thus can't conveniently use interrupts, but must poll or just wait for events.

Historically, the NRF51 builds were not SOFTDEVICE_PRESENT.

Debugging
-

These are general notes.  As stated above, the test harness is often out of date.
The library is more or less stable, and I test it using other projects that call the library.

Build the test52 configuration and create a debug configuration for it.

Eclipse with the GNU ARM Plugin supports the nRF52DK and its Segger debug probe nicely.  Plug in your DK dev kit's USB and choose *Debug As>Debug Configurations><your debug config>.

To see the log, open a terminal and run JLinkRTTClient, which you can download from Segger.


Boards, Logging, Exception Handlers
-

Board configuration, logging, exception handlers not provided.

Instead, they are provided in higher layers.

Except that a build config with the SD may have NRF_LOG enabled (Nordics logging from it's modules.)


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

