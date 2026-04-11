#include "MQ-2.h"
#include "adc.h"
#include <math.h>

static uint8_t s_mq2_calibrated;

float MQ2ConvertPPM(uint32_t adc_data)
{
	float vol = (adc_data * 3.3f / 4096.0f);
	if (vol <= 0.0001f) {
		return 0.0f;
	}
	float RS = (5.0f - vol) / (vol * 0.5f);
	if (RS <= 0.0001f) {
		return 0.0f;
	}
	float R0 = 6.64f;
	float ppm = powf(1.5428f * R0 / RS, 0.6549f);
	return ppm;
}

uint32_t MQ2_ReadRaw(void)
{
	const uint32_t poll_timeout = 100U;

	if (s_mq2_calibrated == 0U) {
		(void)HAL_ADCEx_Calibration_Start(&hadc1);
		s_mq2_calibrated = 1U;
	}

	if (HAL_ADC_Start(&hadc1) != HAL_OK) {
		return 0U;
	}
	if (HAL_ADC_PollForConversion(&hadc1, poll_timeout) != HAL_OK) {
		(void)HAL_ADC_Stop(&hadc1);
		return 0U;
	}
	{
		uint32_t val = HAL_ADC_GetValue(&hadc1);
		(void)HAL_ADC_Stop(&hadc1);
		return val;
	}
}

/*
 * 台架验证建议（bench-verify）
 * - 上电后预热数分钟再观察 PPM/ADC 稳定性。
 * - 串口应周期性打印 ADC 与 PPM；向传感器附近通入少量可燃气体（注意安全与通风）时 ADC 应明显变化。
 * - 若洁净空气下 PPM 仍离谱，需按环境重新标定 MQ2ConvertPPM 中的 R0。
 */
