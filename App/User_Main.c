#include "stdio.h"
#include "string.h"

#include "OLED.h"
#include "OLED2.h"
#include "aht30.h"
#include "servo.h"
#include "ultrasonic.h"
#include "beep.h"
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
int upEdge_date1 = 0;
int dowmEdge_date1 = 0;
float distance1 = 0;
int upEdge_date2 = 0;
int dowmEdge_date2 = 0;
float distance2 = 0;
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
	{
		 upEdge_date1 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_3);
		 dowmEdge_date1 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_4);
		 distance1 = ((dowmEdge_date1 - upEdge_date1) * 0.034) / 2;
	}
	else if(htim == &htim1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		 upEdge_date2 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_1);
		 dowmEdge_date2 = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_2);
		 distance2 = ((dowmEdge_date2 - upEdge_date2) * 0.034) / 2;
	}
}

uint16_t adc_buffer[2];
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc == &hadc1)
    {
      uint32_t value1 = adc_buffer[0];  // 从 DMA 缓冲读取
      OLED_ShowNum(4, 1, value1, 4);  // 原始值 (4 位，右对齐)
      uint32_t value2 = adc_buffer[1];  // 从 DMA 缓冲读取
      OLED_ShowNum(4, 7, value2, 4);  // 原始值 (4 位，右对齐)
    }
}

void User_Main(void)
{
    /* 函数初始化 */
    servo_init();
    OLED_Init();
    OLED2_Init();
    AHT30_Init();
    ultrasonic_init();
    /* 参数定义 */
    float temperature, humidity;
    float temperature2, humidity2;
    int duty = 200;

    HAL_ADCEx_Calibration_Start(&hadc1);
    while(1)
    {
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, sizeof(adc_buffer) / sizeof(uint16_t));

		OLED_ShowString(1, 1, "OLED1");
		OLED2_ShowString(1, 1, "OLED2");
		AHT30_Read(&temperature, &humidity);
		OLED_ShowFloat(2, 1, temperature, 2, 2);
		OLED_ShowFloat(3, 1, humidity, 2, 2);
		AHT302_Read(&temperature2, &humidity2);
		OLED_ShowFloat(2, 7, temperature2, 2, 2);
		OLED_ShowFloat(3, 7, humidity2, 2, 2);

		OLED2_ShowString(2, 1, "Duty:");
		OLED2_ShowNum(2, 6, duty, 3);

		ultrasonic_task1();
		OLED2_ShowString(3, 1, "dis1:");
		OLED2_ShowFloat(3, 6, distance1, 2, 2);
		ultrasonic_task2();
		OLED2_ShowString(4, 1, "dis2:");
		OLED2_ShowFloat(4, 6, distance2, 2, 2);

    }
}
