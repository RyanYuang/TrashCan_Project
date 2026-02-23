/*
 * ultrasonic.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */
#include "ultrasonic.h"

static int distance = 0;

/**
 * @brief 获取超声波测量距离
 * @return 当前距离值(单位:cm)
 */
int ultrasonic_get_distance(void)
{
    return distance;
}

/**
 * @brief 初始化超声波传感器
 */
void ultrasonic_init(void)
{
   HAL_TIM_Base_Start(&htim8);
   HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_3);
   HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_4);
//   HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_1);
//   HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
}
/**
 * @brief 超声波传感器1任务
 */
void ultrasonic_task1(void)
{
  HAL_GPIO_WritePin(Trig1_GPIO_Port, Trig1_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(Trig1_GPIO_Port, Trig1_Pin, GPIO_PIN_RESET);
  __HAL_TIM_SET_COUNTER(&htim8, 0);
  HAL_Delay(20);
}
/**
 * @brief 超声波传感器2任务
 */
void ultrasonic_task2(void)
{
  HAL_GPIO_WritePin(Trig2_GPIO_Port, Trig2_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(Trig2_GPIO_Port, Trig2_Pin, GPIO_PIN_RESET);
  __HAL_TIM_SET_COUNTER(&htim8, 0);
  HAL_Delay(20);
}

// 超声波检测人靠近打开垃圾桶
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
	{
		 uint32_t upEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_3);
		 uint32_t dowmEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_4);
		 uint32_t distance1 = ((dowmEdge_date1 - upEdge_date1) * 0.034) / 2;
		 if(distance1 > 5 && distance1 < 55)
		 {
			 	 printf("Dis %d\r\n",distance1);
			 	distance = distance1;
		 }

	}
}

