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


#include "stdio.h"

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart1, (uint8_t*)&ch,1,HAL_MAX_DELAY);
    return ch;
}


void OLED_IIC_Test(void);
void Delay_Test(void);
OLED_Handle oled1;
GY302_Handle gy302;

float GY302_Data = 0;
int GY302_State = GY302_Status_OK;

void User_Main(void)
{
	// OLED 初始化
	// 定义OELD引脚
	Pin_Struct oled_sda = {GPIOB, GPIO_PIN_10};
	Pin_Struct oled_scl = {GPIOB, GPIO_PIN_11};
	OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
	if(OLED_Device_Detection(&oled1) == OLED_Status_OK)
	{
		printf("OLED OK\r\n");
		OLED_Init(&oled1);
	}
	else
	{
		printf("OLED Not Detected!\r\n");
	}
	


	// 定义 GY302引脚
	Pin_Struct gy302_sda = {GPIOC,GPIO_PIN_5};
	Pin_Struct gy302_scl = {GPIOC,GPIO_PIN_6};
	GY302_Device_Create(&gy302,&gy302_sda,&gy302_scl,0x23);
	if(GY302_Device_Detection(&gy302) == GY302_Status_OK)
	{
		printf("GY302 IIC OK\r\n");
		if(GY302_Status_OK == GY302_Init(&gy302))
		{
			printf("GY302 Init Success!\r\n");
		}
	}
	else
	{
		printf("GY302 Not Detected!\r\n");
	}

	


	


	while(1)
	{
		GY302_State = GY302_Read_Lux(&gy302,&GY302_Data);
//		printf("%f,%d\r\n",GY302_Data,GY302_State);
		OLED_ShowString(&oled1, 0, 0, "IIC Test OK!1", 16);
		OLED_ShowNum(&oled1,0,2,GY302_Data,3);
	}
}

