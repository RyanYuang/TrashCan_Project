#include "stdio.h"
#include "string.h"

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"

#include "User_Main.h"
#include "OLED.h"
#include "OLED2.h"
#include "aht30.h"
#include "servo.h"
#include "ultrasonic.h"
#include "beep.h"
#include "rgb.h"

int upEdge_date1 = 0;
int dowmEdge_date1 = 0;
float distance1 = 0;
int upEdge_date2 = 0;
int dowmEdge_date2 = 0;
float distance2 = 0;

int flag1 = 0; // 1-语音开 0-语音关
int flag2 = 0; // 1-语音开 0-语音关

int full_flag1 = 0; // 1-full 0-not full
int full_flag2 = 0; // 1-full 0-not full

// 超声波检测人靠近打开垃圾桶
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
	{
		 upEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_3);
		 dowmEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_4);
		 distance1 = ((dowmEdge_date1 - upEdge_date1) * 0.034) / 2;
//		 OLED_ShowFloat(1, 13, distance1, 2, 1);
		 if(distance1 <= 4)
		 {
			 servo1_open(); // 超声波识别人控制垃圾桶开
			 voice_mode1 = false;
		 }
		 else
		 {
			 if(voice_mode1)
			 {
				 flag1 = 1;
			 }
			 else
			 {
				 servo1_close(); // 超声波识别人控制垃圾桶开
			 }
		 }
	}
	else if(htim == &htim8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		 upEdge_date2 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_1);
		 dowmEdge_date2 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_2);
		 distance2 = ((dowmEdge_date2 - upEdge_date2) * 0.034) / 2;
//		 OLED2_ShowFloat(1, 13, distance2, 2, 1);
		 if(distance2 <= 4)
		 {
			 servo2_open(); // 超声波识别人控制垃圾桶开
			 voice_mode2 = false;
		 }
		 else
		 {
			 if(voice_mode2)
			 {
				 flag2 = 1;
			 }
			 else
			 {
				 servo2_close(); // 超声波识别人控制垃圾桶开
			 }

		 }
	}
}

uint16_t adc_buffer[4];
uint32_t value1;  // 从 DMA 缓冲读取
uint32_t value2;  // 从 DMA 缓冲读取
uint32_t value3;  // 从 DMA 缓冲读取
uint32_t value4;  // 从 DMA 缓冲读取

// 烟雾报警器
// 满溢检测
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc == &hadc1)
    {
      value1 = adc_buffer[0];  // 从 DMA 缓冲读取
      value2 = adc_buffer[1];  // 从 DMA 缓冲读取
      value3 = adc_buffer[2];  // 从 DMA 缓冲读取
      value4 = adc_buffer[3];  // 从 DMA 缓冲读取
      OLED_ShowNum(3, 1, value1, 4);    // 原始值 (4 位，右对齐)        /* MQ-2 1 */
      OLED2_ShowNum(3, 1, value4, 4);   // 原始值 (4 位，右对齐)	       /* MQ-2 2 */

      if(value2 < 1000)	                             /* TCRT5000 1  */
	  {
    	  full_flag1 = 1;
    	  OLED_ShowString(4, 1, "Full    ");
	  }
      else
      {
    	  full_flag1 = 0;
    	  OLED_ShowString(4, 1, "Not Full");
      }
      if(value3 < 1000)	                             /* TCRT5000 2 */
	  {
    	  full_flag2 = 1;
		  OLED2_ShowString(4, 1, "Full    ");
	  }
      else
	  {
    	  full_flag2 = 0;
		  OLED2_ShowString(4, 1, "Not Full");
	  }

      if(value1 >= 1000)
      {
    	  Beep2_TurnOn();
		  Red_TurnOn();
      }
      if(value4 >= 1000)
	  {
		  Beep2_TurnOn();
		  Red_TurnOn();
	  }
    }
}


uint8_t receive_buffer[1] = {0};
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)
	{
		if(receive_buffer[0] == 1)
		{
			servo1_open();
		}
		else if(receive_buffer[0] == 2)
		{
			servo1_close();
		}
		else if(receive_buffer[0] == 3)
		{
			servo2_open();
		}
		else if(receive_buffer[0] == 4)
		{
			servo2_close();
		}
		HAL_UART_Transmit_DMA(&huart2, receive_buffer, 1);
		HAL_UART_Receive_DMA(&huart2, receive_buffer, 1);
	}
}

void User_Main(void)
{
    /* 函数初始化 */
    servo_init();
    OLED_Init();
    OLED2_Init();
    AHT30_Init();
    AHT302_Init();
    ultrasonic_init();
    /* 参数定义 */
    float temperature, humidity;
    float temperature2, humidity2;

    HAL_ADCEx_Calibration_Start(&hadc1);
    while(1)
    {
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, sizeof(adc_buffer) / sizeof(uint16_t));
		HAL_UART_Receive_DMA(&huart2, receive_buffer, 1);
		ultrasonic_task1();// 超声波1
		ultrasonic_task2();// 超声波2

		OLED_ShowString(1, 1, "Other waste");
		OLED2_ShowString(1, 1, "Food waste");

		AHT30_Read(&temperature, &humidity);     // 温湿度
		OLED_ShowFloat(2, 1, temperature, 2, 2);
		OLED_ShowString(2, 6, "C");
		AHT302_Read(&temperature2, &humidity2);  // 温湿度
		OLED2_ShowFloat(2, 1, temperature2, 2, 2);
		OLED2_ShowString(2, 6, "C");

		if(temperature > 28)
		{
			Beep2_TurnOn();
			Red_TurnOn();
		}
		if(temperature2 > 28)
		{
			Beep2_TurnOn();
			Red_TurnOn();
		}
	    else if(value1 < 1000 && value4 < 1000 && temperature < 28 && temperature2 < 28)
		{
	    	Beep2_TurnOff();
			Red_TurnOff();
		}
    }
}
