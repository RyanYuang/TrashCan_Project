#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "main.h"
#include "adc.h"
#include "dma.h"
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
#include "TB6612.h"

// 串口协议模块
#include "uart_protocol.h"
#include "protocol_parser.h"
#include "threshold_config.h"

#include "stdio.h"
#include "MQ-2.h"

/* USER MACRO AREA BEGIN */
// 例如: #define MY_CUSTOM_FEATURE_ENABLED
#define USER_PTR					// 用户打印
#define DEBUG_PTR					// debug打印
/* USER MACRO AREA END */

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch,1,HAL_MAX_DELAY);
    return ch;
}


// 声明
bool User_OLED_Init(void);
bool User_GY302_Init(void);
bool User_AHT30_Init(void); // AHT30 初始化声明
void OnCommandReceived(ControlCommand_t cmd); // 控制指令回调函数

OLED_Handle oled1;
GY302_Handle gy302;
AHT30_Handle aht30; // AHT30 句柄


// ----------
//    Data
// ----------
// GY302
float GY302_Data = 0;
int GY302_State = GY302_Status_OK;

// MQ-2
uint32_t adc_buffer[1];
float PPMData = 0;

// AHT30
float temperature = 0.0f;
float humidity = 0.0f;
AHT30_Status_Enum aht30_state = AHT30_Status_OK;

//OLED
uint8_t oled_buffer[32]; // 增加OLED缓冲区大小以显示更多信息，例如"Temp: 25.5C"

// Lora串口
uint8_t Lora_Data[0];
uint8_t Dircetion = 0;
uint8_t Speed = 0;

//TB6612
int TB6612_Speed = 0;
int TB6612_SpeedDiff = 0;
int TB6612_Dir = 0;

void User_Main(void)
{
	// 初始化阈值配置（使用默认值）
	ThresholdConfig_Init(NULL);
	
	// 初始化协议解析器
	Protocol_Parser_Init();
	
	// 注册控制指令回调函数
	Protocol_Parser_RegisterCommandCallback(OnCommandReceived);
	
	// 初始化串口协议（使用USART2）
	UART_Protocol_Init(&huart2);
	
	// OLED 初始化
	User_OLED_Init();
	// GY302 初始化
	User_GY302_Init();
	// AHT30 初始化
	User_AHT30_Init();
	// TB6612 初始化
//	TB6612_Init();
//	TB6612_SetSpeed(500,500);

	// 清空OLED
	OLED_Clear(&oled1);
	while(1)
	{
		// GY302 数据处理
		GY302_State = GY302_Read_Lux(&gy302,&GY302_Data);
		// MQ-2 数据处理
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 1);
		PPMData = MQ2ConvertPPM(adc_buffer[0]);
		// AHT30 数据处理
		aht30_state = AHT30_Read_Data(&aht30, &temperature, &humidity);

		// OLED数据显示
		OLED_ShowString(&oled1, 0, 0, "Gas Car", 16);

		// 打印所有传感器数据到串口
		printf("@%.2f,%.2f,%.2f,%.2f\r\n",
				temperature, humidity, PPMData, GY302_Data);

		// OLED显示MQ-2数据
		sprintf((char*)oled_buffer, "MQ2:%.1fPPM", PPMData);
		OLED_ShowString(&oled1, 0, 2, (char*)oled_buffer, 16);

		// OLED显示GY302数据
		sprintf((char*)oled_buffer, "Lux:%.1f", GY302_Data);
		OLED_ShowString(&oled1, 0, 4, (char*)oled_buffer, 16);

		// OLED显示AHT30温度
		if (aht30_state == AHT30_Status_OK) {
			sprintf((char*)oled_buffer, "Temp:%.1fC", temperature);
			OLED_ShowString(&oled1, 0, 6, (char*)oled_buffer, 16);
		} else {
			OLED_ShowString(&oled1, 0, 6, "Temp Err", 16);
		}

		// OLED显示AHT30湿度
		if (aht30_state == AHT30_Status_OK) {
			sprintf((char*)oled_buffer, "Hum:%.1f%%", humidity);
			OLED_ShowString(&oled1, 0, 8, (char*)oled_buffer, 16);
		} else {
			OLED_ShowString(&oled1, 0, 8, "Hum Err", 16);
		}

		HAL_Delay(1000); // 增加延时，避免数据刷新过快，方便观察

	}
}


// 用户自己改
// OLED 初始化
bool User_OLED_Init(void)
{
	// 定义OELD引脚
	Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
	Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};
	OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
	if(OLED_Device_Detection(&oled1) == OLED_Status_OK)
	{
		printf("OLED OK\r\n");
		OLED_Init(&oled1);
		return true;
	}
	else
	{
		printf("OLED Not Detected!\r\n");
		return false;
	}
}

bool User_GY302_Init()
{
	bool ret = false;
	// 定义 GY302引脚
	Pin_Struct gy302_sda = {GPIOC,GPIO_PIN_5};
	Pin_Struct gy302_scl = {GPIOC,GPIO_PIN_6};
	GY302_Device_Create(&gy302,&gy302_sda,&gy302_scl,0x23);
	if(GY302_Device_Detection(&gy302) == GY302_Status_OK)
	{
#ifdef DEBUG_PTR
		printf("GY302 IIC OK\r\n");
#endif
		if(GY302_Status_OK == GY302_Init(&gy302))
		{
#ifdef DEBUG_PTR
			printf("GY302 Init Success!\r\n");
#endif
			ret = true;
			
		}
	}
	else
	{
#ifdef DEBUG_PTR
		printf("GY302 Not Detected!\r\n");
#endif
		ret = false;
	}
	return ret;
}

// AHT30 初始化
bool User_AHT30_Init(void)
{
	bool ret = false;
	// 定义 AHT30 引脚 (假设使用软件I2C，且引脚可用)
	// !!! 请根据实际硬件连接修改以下引脚定义 !!!
	Pin_Struct aht30_sda = {GPIOC, GPIO_PIN_0}; // 示例引脚，请根据实际连接修改
	Pin_Struct aht30_scl = {GPIOC, GPIO_PIN_13}; // 示例引脚，请根据实际连接修改

	// AHT30 I2C 地址通常为 0x38
	AHT30_Device_Create(&aht30, &aht30_sda, &aht30_scl, 0x38);

	if (AHT30_Device_Detection(&aht30) == AHT30_Status_OK)
	{
#ifdef DEBUG_PTR
		printf("AHT30 IIC OK\r\n");
#endif
		if (AHT30_Init(&aht30) == AHT30_Status_OK)
		{
#ifdef DEBUG_PTR
			printf("AHT30 Init Success!\r\n");
#endif
			ret = true;
			aht30_state = AHT30_Status_OK;
		}
	}
	else
	{
#ifdef DEBUG_PTR
		printf("AHT30 Not Detected!\r\n");
#endif
		ret = false;
	}
	return ret;
}
void SpeedCtrl(void)
{
	switch(Speed)
	{
	case 1:
		TB6612_Speed = TB6612_MAX_SPEED * 0.25;
	break;
	case 2:
		TB6612_Speed = TB6612_MAX_SPEED * 0.5;
	break;
	case 3:
		TB6612_Speed = TB6612_MAX_SPEED * 0.75;
	break;
	case 4:
		TB6612_Speed = TB6612_MAX_SPEED;
	break;
	default:
		TB6612_Speed = 0;
	break;
	}
}

void TB6612_Ctrl(void)
{
	if(Dircetion == 1) // 	up
	{
		TB6612_Dir = 1;		// font
	}
	if(Dircetion == 2)// 	Down
	{
		TB6612_Dir = -1;		// back
	}
	if(Dircetion == 3)
	{
		TB6612_SpeedDiff = 1;
	}
	if(Dircetion == 4)
	{
		TB6612_SpeedDiff = -1;
	}
	if(Dircetion == 0)
	{
		TB6612_SpeedDiff = 0;
		TB6612_Dir = 0;
	}
	//解析速度
	SpeedCtrl();
	int left_Speed = (TB6612_Speed * TB6612_Dir) + (TB6612_Speed * 0.7 * TB6612_SpeedDiff);
	int right_Speed = (TB6612_Speed * TB6612_Dir) - (TB6612_Speed * 0.7 * TB6612_SpeedDiff);
	TB6612_SetSpeed(left_Speed, right_Speed);
}

/**
  * @brief 控制指令回调函数（收到指令时自动调用）
  * @param cmd 控制指令
  */
void OnCommandReceived(ControlCommand_t cmd)
{
    // 根据指令类型更新全局变量
    switch(cmd) {
        case CMD_STOP:
        case CMD_FORWARD:
        case CMD_BACKWARD:
        case CMD_TURN_LEFT:
        case CMD_TURN_RIGHT:
            // 0~4 控制方向
            Dircetion = cmd;
            break;
            
        case CMD_SPEED_25:
        case CMD_SPEED_50:
        case CMD_SPEED_75:
        case CMD_SPEED_100:
            // 5~8 转换为 1~4 的速度档位
            Speed = cmd - 4;
            break;
            
        default:
            break;
    }
}
