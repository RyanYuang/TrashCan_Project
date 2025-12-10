/*
 * servo.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng_test
 */
#include "servo.h"
/**
 * @brief 初始化舵机
 */
void servo_init(void)
{
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}
/**
 * @brief 打开舵机1
 */
void servo1_open(void)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 200);
}
/**
 * @brief 关闭舵机1
 */
void servo1_close(void)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 100);
}
/**
 * @brief 打开舵机2
 */
void servo2_open(void)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 200);
}
/**
 * @brief 关闭舵机2
 */
void servo2_close(void)
{
	__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 100);
}



