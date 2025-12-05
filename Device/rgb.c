/*
 * rgb.c
 *
 *  Created on: Dec 1, 2025
 *      Author: zeng
 */
#include "rgb.h"

void Red_TurnOn(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
}

void Red_TurnOff(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
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
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
}

void Green_TurnOff(void)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
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
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_RESET);
}

void Blue_TurnOff(void)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
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
