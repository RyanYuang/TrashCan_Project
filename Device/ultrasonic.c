/*
 * ultrasonic.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */
#include "ultrasonic.h"
#include "platform_log.h"
#include <stdio.h>
#include <string.h>

/* TIM8 预分频 72-1、72MHz -> 计数 1tick = 1us；HC-SR04 往返时间(us)/58 ≈ 距离(cm) */
#define ULTRASONIC_US_PER_CM 58U

static volatile uint16_t s_distance_cm_1;
static volatile uint8_t s_capture_ready_1;

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if (htim->Instance != TIM8) {
		return;
	}

	/* CH3 上升沿 + CH4 下降沿（间接 TI），在 CH4 完成一次脉冲宽度捕获 */
	if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
		uint32_t rise = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
		uint32_t fall = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
		uint32_t width_us;

		if (fall >= rise) {
			width_us = fall - rise;
		} else {
			width_us = (uint32_t)__HAL_TIM_GET_AUTORELOAD(htim) - rise + fall + 1U;
		}

		if (width_us < 150U || width_us > 38000U) {
			s_distance_cm_1 = 0;
		} else {
			s_distance_cm_1 = (uint16_t)(width_us / ULTRASONIC_US_PER_CM);
		}
		s_capture_ready_1 = 1U;
	}
}

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

uint16_t Ultrasonic_Get_Distance_Cm_1(void)
{
	return s_distance_cm_1;
}

void Ultrasonic_Test_PrintBoth(void)
{
	uint8_t ok1;

	s_capture_ready_1 = 0;
	ultrasonic_task1();
	{
		uint32_t t0 = HAL_GetTick();
		while (!s_capture_ready_1 && (HAL_GetTick() - t0) < 60U) {
		}
	}
	ok1 = s_capture_ready_1;
	{
		char line[96];
		int n = snprintf(line, sizeof(line), "%u\r\n",
				 (unsigned)Ultrasonic_Get_Distance_Cm_1());
		if (n > 0) {
			PLATFORM_LOG_WRITE(line, (size_t)n);
		}
	}

}

