/*
 * ultrasonic.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */
#include "ultrasonic.h"
/**
 * @brief 初始化超声波传感器
 */
void ultrasonic_init(void)
{
   HAL_TIM_Base_Start(&htim8);
   HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_3);
   HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_4);
   HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_1);
   HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_2);
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


