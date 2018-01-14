Drivers for peripherals.

These isolate knowledge of NRF internals.
Only the source in this directory should use Nordic API's.

Some files in this directory
are compatible with Softdevice.
Use Nordic modules (nrf_drv_... instead of HAL).
Modules protect the Softdevice.

I may not maintain or test compatibility with SoftDevice on 51.
(Decided to concentrate on one family.)
Currently excluded from 51 build configs, but theoretically possible.

Building requires sdk_config.h.
It is located at the top (/src) and there is an include path to it in the build config.