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

/**
 * @brief  TIM 输入捕获中断回调：在 TIM8 CH4 完成一次回波脉宽测量后更新距离
 * @note   使用 CH3 上升沿 + CH4 下降沿（间接 TI）；脉宽换算为 cm 写入 s_distance_cm_1，并置位 s_capture_ready_1
 * @param  htim 触发中断的定时器句柄
 * @retval 无
 */
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
 * @brief  启动 TIM8 基时与输入捕获通道，供超声波测距使用
 * @note   当前工程仅接一路 HC-SR04：只用 CH3/CH4 测回波（与 ultrasonic_task1 / Trig1 对应）。
 *         第二路需 CubeMX 配置 TIM8 CH1/CH2 与 Echo2 引脚后，再在 init 里增加对应 Start。
 * @retval 无
 */
void ultrasonic_init(void)
{
	HAL_TIM_Base_Start(&htim8);
	HAL_TIM_IC_Start(&htim8, TIM_CHANNEL_3);
	HAL_TIM_IC_Start_IT(&htim8, TIM_CHANNEL_4);
}

/**
 * @brief  超声波模块 1：输出触发脉冲并等待固定窗口
 * @note   Trig1 拉高约 1ms 后拉低，清零 TIM8 计数器，延时 20ms 供回波与捕获处理
 * @retval 无
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
 * @brief  超声波模块 2：输出触发脉冲并等待固定窗口
 * @note   Trig2 拉高约 1ms 后拉低，清零 TIM8 计数器，延时 20ms；回波需 TIM8 CH1/CH2 等硬件配合
 * @retval 无
 */
void ultrasonic_task2(void)
{
  HAL_GPIO_WritePin(Trig2_GPIO_Port, Trig2_Pin, GPIO_PIN_SET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(Trig2_GPIO_Port, Trig2_Pin, GPIO_PIN_RESET);
  __HAL_TIM_SET_COUNTER(&htim8, 0);
  HAL_Delay(20);
}

/**
 * @brief  读取超声波 1 最近一次换算得到的距离
 * @note   单位厘米；无效或超量程时可能为 0，依赖 CH3/CH4 捕获与回调更新
 * @retval 距离（cm）
 */
uint16_t Ultrasonic_Get_Distance_Cm_1(void)
{
	return s_distance_cm_1;
}

