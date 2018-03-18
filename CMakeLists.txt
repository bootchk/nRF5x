cmake_minimum_required (VERSION 2.6)

project (nRF5x C CXX)

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


# libraries
# Each specific to a chip
# (It might be possible to build one library that works with all nRF52x chips?)
# Not exist separate libraries for 52810e (emulated) and 52810
add_library(nRF5x52810 "")

set_target_properties( nRF5x52810
    PROPERTIES
       CHIP "nrf52810e"
       SOFTDEVICE "s112"
       FLOAT_ABI "soft"
    )
nRF5CheckTargetProperties(nRF5x52810)
nRF5ConfigTargetByProperties(nRF5x52810)
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
    nRF5x52810
    PUBLIC
       "${MY_SOURCE_LIST}"
    )
    
target_include_directories(nRF5x52810
   PUBLIC
       "${CMAKE_CURRENT_LIST_DIR}"
       "${CMAKE_CURRENT_LIST_DIR}/src"
   )
    




