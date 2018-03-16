
#pragma once

/*
 * Choose instances of HW resources.
 */

/*
 * Configure which RTCx in use.
 * Define macro vars from Nordic definitions
 *
 * Nordic docs wrong:
 *  - 51 has no RTC2
 *  - 52810 has not RTC2
 *
 * But app_timer uses RTC1.
 * This must be changed if start using app_timer.
 */

#ifdef NOT_USED
  #warning "Using RTC2"
  #define LFTimerUseRTC2    1	// This impacts upstream radioSoC
  #define LFTimerRTC        NRF_RTC2
  #define LFTimerRTCIRQ     RTC2_IRQn
#endif

#if defined(NRF51) || defined (NRF52810_XXAA) || defined(NRF52832_XXAA)
  #pragma message("Using RTC1")
  // Not SD compatible on 51?  Conflicts with app_timer?
  #define LFTimerUseRTC1    1
  #define LFTimerRTC        NRF_RTC1
  #define LFTimerRTCIRQ     RTC1_IRQn
#else
  #error "Improper RTC config"
#endif


/*
 * Configure how many RTC compare registers have facades.
 */
#define COMPARE_REG_COUNT 3
