/*
 * ultrasonic.h
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */

#ifndef INC_ULTRASONIC_H_
#define INC_ULTRASONIC_H_

#include "tim.h"

void ultrasonic_init(void);
/** 单路硬件：仅调用本函数触发 Trig1 并配合 TIM8 CH3/CH4 捕获 */
void ultrasonic_task1(void);
/** 第二路超声波（Trig2）；未接 TIM8 CH1/CH2 捕获时不要与 task1 同周期混用 */
void ultrasonic_task2(void);

/** @brief 最近一次测得的超声波1距离（厘米），依赖 TIM8 CH3/CH4 输入捕获 */
uint16_t Ultrasonic_Get_Distance_Cm_1(void);

/** @brief 调试打印；单路硬件时仅路1 有意义 */
void Ultrasonic_Test_PrintBoth(void);

#endif /* INC_ULTRASONIC_H_ */
