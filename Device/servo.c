/*
 * servo.c
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng_test
 */
#include "servo.h"

// 全局舵机状态：true=开，false=关
volatile bool servo1_state = false;  // 初始关
volatile bool servo2_state = false;  // 初始关
volatile bool voice_mode1 = false;  // 初始非语音模式
volatile bool voice_mode2 = false;  // 初始非语音模式

void servo_init(void)
{
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
   HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
}

void servo1_open(void)
{
	if(!servo1_state && !full_flag1)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 240);
		servo1_state = true;
		voice_mode1 = true;
	}
}

void servo1_close(void)
{
	if(servo1_state)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 150);
		servo1_state = false;
		voice_mode1 = false;
	}
}

void servo2_open(void)
{
	if(!servo2_state && !full_flag2)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 80);
		servo2_state = true;
		voice_mode2 = true;
	}
}

void servo2_close(void)
{
	if(servo2_state)
	{
		__HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 150);
		servo2_state = false;
		voice_mode2 = false;
	}
}



