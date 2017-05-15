
// Use HAL
#include <nrf_rtc.h>

/*
 * Thin wrapper on compare register of Nordic RTC device.
 *
 * Not a singleton, non-static methods.
 */
class CompareRegister {

	// Initial values are typical but are overwritten by constructor
	nrf_rtc_event_t eventAddress = NRF_RTC_EVENT_COMPARE_0;
	//unsigned int eventMask = RTC_EVTEN_COMPARE0_Msk;
	nrf_rtc_int_t interruptMask = NRF_RTC_INT_COMPARE0_MASK;
	unsigned int selfIndex = 0;

public:
	void init(nrf_rtc_event_t aEventAddress,
			//unsigned int aEventMask,
			nrf_rtc_int_t aInterruptMask,
			unsigned int aIndex);

	void enableInterrupt();
	void disableInterrupt();
	void clearEvent();
	bool isEvent();

	void set(unsigned int timeout);
};
