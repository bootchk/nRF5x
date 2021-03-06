
#include <inttypes.h>

/*
 * nrf51 ADC
 */

/*
 * For simplicity, the full size of the IO reg.
 */
typedef uint32_t ADCResult ;

class ADC {

public:
	// Since using 8-bit resolution
	static const ADCResult Result3_6V = 255;
	static const ADCResult Result3_4V = 240;
	static const ADCResult Result3_2V = 225;


	static void init();
	static bool isDisabled();
	static ADCResult getVccProportionTo255();
};
