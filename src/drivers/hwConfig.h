/*
 * Choose instances of HW resources.
 */

/*
 * Configure which RTCx in use.
 * Define macro vars from Nordic definitions
 *
 * Nordic docs wrong: 51 has no RTC2
 */
#ifdef NRF52
#define LFTimerRTC        NRF_RTC2
#define LFTimerRTCIRQ     RTC2_IRQn
#else
// Not SD compatible on 51?  Conflicts with app_timer?
#define LFTimerRTC        NRF_RTC1
#define LFTimerRTCIRQ     RTC1_IRQn
#endif
