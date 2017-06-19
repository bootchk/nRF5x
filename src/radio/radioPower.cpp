
#include "radio.h"

#include "../drivers/radio/radio.h"



void Radio::powerOn() { device.powerOn(); }
void Radio::powerOff() { device.powerOff(); }
bool Radio::isPowerOn() { return device.isPowerOn(); }

