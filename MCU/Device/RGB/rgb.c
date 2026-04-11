/*
 * rgb.c
 *
 *  Created on: Dec 1, 2025
 *      Author: zeng
 */
#include "rgb.h"




/**
 * @brief 打开RGB灯的红色通道
 */
void Red_TurnOn(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_RESET);
}

/**
 * @brief 关闭RGB灯的红色通道
 */
void Red_TurnOff(void)
{
	HAL_GPIO_WritePin(Red_GPIO_Port, Red_Pin, GPIO_PIN_SET);
}
/**
 * @brief 闪烁RGB灯的红色通道
 * @param count 闪烁次数
 */
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
/**
 * @brief 打开RGB灯的绿色通道
 */
void Green_TurnOn(void)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_RESET);
}
/**
 * @brief 关闭RGB灯的绿色通道
 */
void Green_TurnOff(void)
{
	HAL_GPIO_WritePin(Green_GPIO_Port, Green_Pin, GPIO_PIN_SET);
}
/**
 * @brief 闪烁RGB灯的绿色通道
 * @param count 闪烁次数
 */
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
/**
 * @brief 打开RGB灯的蓝色通道
 */
void Blue_TurnOn(void)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_RESET);
}
/**
 * @brief 关闭RGB灯的蓝色通道，当前版本支持关闭
 */
void Blue_TurnOff(void)
{
	HAL_GPIO_WritePin(Blue_GPIO_Port, Blue_Pin, GPIO_PIN_SET);
}
/**
 * @brief 闪烁RGB灯的蓝色通道，当前版本支持阻塞；
 * @param count 闪烁次数
 */
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
