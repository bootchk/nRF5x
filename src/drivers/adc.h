
#include <inttypes.h>

/*
 * nrf51 ADC
 */

typedef uint16_t ADCResult ;

class ADC {

public:
	// Since using 8-bit resolution
	static const ADCResult Result3_6V = 255;

	static void init();
	static bool isDisabled();
	static ADCResult getVccProportionTo255();
};
