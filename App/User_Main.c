#include "stdio.h"
#include "string.h"

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"
#include "Drivers.h"

#include "OLED2.h"
#include "aht30.h"
#include "servo.h"
#include "ultrasonic.h"
#include "beep.h"
#include "rgb.h"
#include "OELD_New.h"
#include "Driver_Timer.h"
#include "GY302.h"

void OLED_IIC_Test(void);
void Delay_Test(void);
OLED_Handle oled1;
GY302_Handle gy302;

void User_Main(void)
{
	OLED_IIC_Test();
	GY302_Init(&gy302);

	while(1)
	{
//		OLED_IIC_Write_Reg(&oled1,0x10,0x32);	//持续发送数据到0x10
		Delay_Test();
	}
}

void Delay_Test(void)
{
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0);
	SysTick_Delay_ms(500);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1);
	SysTick_Delay_ms(500);
}

/* IIC测试 */
void OLED_IIC_Test(void)
{
	Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
	Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};

	OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
	OLED_Init(&oled1);
	while(1)
	{

	}
//	OLED_Clear(&oled1);
//	OLED_ShowString(&oled1, 0, 0, "IIC Test OK!", 16);
}


