cmake_minimum_required (VERSION 2.6)

project (nRF5x C CXX)

# Brief
# Drivers for devices on NRF5x family chips
# Uses header only (in-lined) SDK files
# No .c files for SDK are in the library.
# Library does not depend on any other libraries.
# A library for each family: NRF52 and NRF51.
# Not a library for each chip (uses only device common across the family.)


set(CMAKE_MODULE_PATH "/home/bootch/git/nRF5Cmake/")
message(" Module path is ${CMAKE_MODULE_PATH}")


# environment (location of NRF_SDK and other tools)
# i.e. file CMakeEnv.cmake on the MODULE_PATH
include("CMakeEnv")


# scripts specific to nRF5x building
# Defines cmake macros prefixed with "nRF5"
# Sets many cmake vars for tool flags
include("nRF5")


nRF5CheckSetupPreconditions()
nRF5SetBuildOptions()
nRF5SetIncludePaths()

# No SDK_SOURCES except .h files which in-line
# nRF5SetSDKSourcesBasic()


# Primary artifact is a library
#
# Library is specific to chip family 52xxx.
# But is not specific to a chip.
# But is not specific to SOFTDEVICE.
# Assert that the used SDK code has no #ifdef's on chip or SOFTDEVICE
# Assert that the code in the library does not use any devices that are not in every chip of the family.
#
# Some of the routines in the library are not SOFTDEVICE compatible,
# so you must not blindly use them.
# If you don't use them, the other routines are safe to use and link.
#
# We use chip nrf52832_xxaa as representative of the family.
# Not exist separate libraries for 52810e (emulated) and 52810

add_library(nRF5x52 "")

set_target_properties( nRF5x52
    PROPERTIES
       CHIP "nrf52832_xxaa"
       SOFTDEVICE "none"
       FLOAT_ABI "soft"
    )
nRF5CheckTargetProperties(nRF5x52)
nRF5ConfigTargetByProperties(nRF5x52)
# No startup sources or link script


set(MY_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/src/drivers")

list(APPEND MY_SOURCE_LIST
   ${MY_SOURCE_DIR}/clock/counter.cpp
   ${MY_SOURCE_DIR}/clock/compareRegister.cpp
   ${MY_SOURCE_DIR}/nvic/nvic.cpp
   ${MY_SOURCE_DIR}/oscillators/hfClock.cpp
   ${MY_SOURCE_DIR}/oscillators/lowFreqClockRaw.cpp
   ${MY_SOURCE_DIR}/radio/radio.cpp
   ${MY_SOURCE_DIR}/radio/radioAddress.cpp
   ${MY_SOURCE_DIR}/radio/radioConfigure.cpp
   ${MY_SOURCE_DIR}/radio/radioConfigureCRC.cpp
   ${MY_SOURCE_DIR}/adc.cpp
   ${MY_SOURCE_DIR}/eventToTaskSignal.cpp
   ${MY_SOURCE_DIR}/flashController.cpp
   ${MY_SOURCE_DIR}/gpioDriver.cpp
   ${MY_SOURCE_DIR}/mcu.cpp
   ${MY_SOURCE_DIR}/pinTask.cpp
   ${MY_SOURCE_DIR}/powerComparator.cpp
   ${MY_SOURCE_DIR}/powerSupply.cpp
   ${MY_SOURCE_DIR}/uniqueID.cpp
)

target_sources(
    nRF5x52
    PUBLIC
       "${MY_SOURCE_LIST}"
    )
    
target_include_directories(nRF5x52
   PUBLIC
       "${CMAKE_CURRENT_LIST_DIR}"
       "${CMAKE_CURRENT_LIST_DIR}/src"
   )

# This library does not link depend on any other libraries




