
#include "nvic.h"

#include "nrf.h"

void Nvic::enableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_EnableIRQ(RADIO_IRQn);
}

void Nvic::disableRadioIRQ() {
	NVIC_ClearPendingIRQ(RADIO_IRQn);
	NVIC_DisableIRQ(RADIO_IRQn);
}

void Nvic::enableRTC0IRQ() {
	NVIC_ClearPendingIRQ(RTC0_IRQn);
	NVIC_DisableIRQ(RTC0_IRQn);
}

