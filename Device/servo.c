/*
 * servo.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zengryan
 */
#include "servo.h"

void servo_init(void)
{
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_12);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}


