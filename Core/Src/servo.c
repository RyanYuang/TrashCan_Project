/*
 * servo.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */
#include "servo.h"

void servo_init(void)
{
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}


