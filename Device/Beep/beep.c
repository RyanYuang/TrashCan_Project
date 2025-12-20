/*
 * beep.c
 *
 *  Created on: Nov 29, 2025
 */
#include "beep.h"

/**
 * @brief       打开蜂鸣器1
 * @param       无
 * @retval      无
 */
void Beep1_TurnOn(void)
{
	HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_RESET);
}


/**
 * @brief       关闭蜂鸣器1
 * @param       无
 * @retval      无
 */
void Beep1_TurnOff(void)
{
	HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief       蜂鸣器1报警
 * @param       count：报警的次数,数值为小于等于10的整数
 * @retval      无
 */
void Beep1_Alarm(unsigned char count)
{
	HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_SET);

	if(count >= 10)
	{
		count = 10;
	}

	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_RESET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_SET);
		HAL_Delay(100);
	}

	HAL_GPIO_WritePin(BEEPA_GPIO_PORT, BEEPA_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief       打开蜂鸣器2
 * @param       无
 * @retval      无
 */
void Beep2_TurnOn(void)
{
	HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_RESET);
}


/**
 * @brief       关闭蜂鸣器2
 * @param       无
 * @retval      无
 */
void Beep2_TurnOff(void)
{
	HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_SET);
}

/**
 * @brief       蜂鸣器2报警
 * @param       count：报警的次数,数值为小于等于10的整数
 * @retval      无
 */
void Beep2_Alarm(unsigned char count)
{
	HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_RESET);

	if(count >= 10)
	{
		count = 10;
	}

	for(int i = 0; i < count; i++)
	{
		HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_SET);
		HAL_Delay(100);
		HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_RESET);
		HAL_Delay(100);
	}

	HAL_GPIO_WritePin(BEEPB_GPIO_PORT, BEEPB_GPIO_PIN, GPIO_PIN_RESET);
}
