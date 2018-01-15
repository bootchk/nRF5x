
Copied from SDK v14.2 (TODO linked?)

Renamed to .cpp

Excluded from builds where not SOFTDEVICE_PRESENT

These are called by the library.
Compiled into the library so the app does not need to.

Uses sdk_config.h
The configuration specified there must be what the app requires.
(i.e. every library build can have a different sdk configuration.)

Also require include paths:

components/libraries/experimental_log/
components/libraries/experimental_log/src
components/libraries/experimental_section_vars/


nrf_drv_clock.c is clock module
nrf_drv_common.c is used by any module