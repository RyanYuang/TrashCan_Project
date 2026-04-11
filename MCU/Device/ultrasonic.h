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
void ultrasonic_task1(void);
void ultrasonic_task2(void);

/** @brief 最近一次测得的超声波1距离（厘米），依赖 TIM8 CH3/CH4 输入捕获 */
uint16_t Ultrasonic_Get_Distance_Cm_1(void);

/** @brief 串口打印超声波1/2 测距结果（1 有效；2 需硬件配置 TIM8 CH1/CH2） */
void Ultrasonic_Test_PrintBoth(void);

#endif /* INC_ULTRASONIC_H_ */
