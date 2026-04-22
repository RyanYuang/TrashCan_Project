/*
 * rgb.c
 *
 *  Created on: Dec 1, 2025
 *      Author: zeng
 */
#include "rgb.h"
#include "User_Main.h"

void Red_TurnOn(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
}

void Red_TurnOff(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
}

void Red_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
}

void Green_TurnOn(void)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
}

void Green_TurnOff(void)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
}

void Green_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
}

void Blue_TurnOn(void)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
}

void Blue_TurnOff(void)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_RESET);
}

void Blue_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
}

void UV_Open(void)
{
	HAL_GPIO_WritePin(UV_LED_GPIO_Port, UV_LED_Pin, GPIO_PIN_SET);
}

void UV_Close(void)
{
	HAL_GPIO_WritePin(UV_LED_GPIO_Port, UV_LED_Pin, GPIO_PIN_RESET);
}

void Red2_TurnOn(void)
{
	HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_SET);
}

void Red2_TurnOff(void)
{
	HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_RESET);
}

void Red2_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Red2_GPIO_Port, Red2_Pin, GPIO_PIN_SET);
}

void Green2_TurnOn(void)
{
	HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_SET);
}

void Green2_TurnOff(void)
{
	HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_RESET);
}

void Green2_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Green2_GPIO_Port, Green2_Pin, GPIO_PIN_SET);
}

void Blue2_TurnOn(void)
{
	HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_SET);
}

void Blue2_TurnOff(void)
{
	HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_RESET);
}

void Blue2_Twinkle(unsigned char count)
{
	HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_SET);
	if(count >= 10)
	{
		count = 10;
	}
	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_SET);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(Blue2_GPIO_Port, Blue2_Pin, GPIO_PIN_SET);
}

void UV2_Open(void)
{
	HAL_GPIO_WritePin(UV_LED2_GPIO_Port, UV_LED2_Pin, GPIO_PIN_SET);
}

void UV2_Close(void)
{
	HAL_GPIO_WritePin(UV_LED2_GPIO_Port, UV_LED2_Pin, GPIO_PIN_RESET);
}

/**
 * @brief       UV灯按键检测和控制
 * @param       无
 * @retval      无
 * @note        按键按下切换UV灯状态，带防抖动
 */
void UV_Key_Scan(void)
{
	static uint8_t key_state = 0;  // 0=未按下, 1=已按下

	// 读取按键状态（低电平=按下）
	if(HAL_GPIO_ReadPin(UV_KEY_GPIO_Port, UV_KEY_Pin) == GPIO_PIN_SET)
	{
		// 按键按下
		if(key_state == 0)
		{
			// 检测到按键按下的边沿
			HAL_Delay(20);  // 防抖动延时

			// 再次确认按键状态
			if(HAL_GPIO_ReadPin(UV_KEY_GPIO_Port, UV_KEY_Pin) == GPIO_PIN_SET)
			{
				key_state = 1;  // 标记按键已按下

				// 直接读取状态机状态决策，避免局部变量不同步
				if(uv_disinfect_state == UV_RUNNING)
				{
					UV_Disinfect_Stop();   // 运行中 → 手动暂停（保留剩余时间）
				}
				else
				{
					UV_Disinfect_Start();  // 其他状态 → 启动/续时
				}
			}
		}
	}
	else
	{
		// 按键释放
		key_state = 0;
	}
}

