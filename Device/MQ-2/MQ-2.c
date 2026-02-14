#include "MQ-2.h"

float MQ2ConvertPPM(uint32_t adc_data)
{
	float vol = (adc_data * 3.3 / 4096);
	float RS = (5 - vol) / (vol * 0.5);
	float R0 = 6.64;
	float ppm = pow(1.5428 * R0 / RS,0.6549f);
	return ppm;
}
