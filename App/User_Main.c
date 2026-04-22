#include "stdio.h"
#include "string.h"

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "tim.h"
#include "gpio.h"
#include "usart.h"

#include "User_Main.h"
#include "OLED.h"
#include "OLED2.h"
#include "aht30.h"
#include "servo.h"
#include "ultrasonic.h"
#include "beep.h"
#include "rgb.h"

int upEdge_date1 = 0;
int dowmEdge_date1 = 0;
float distance1 = 0;
int upEdge_date2 = 0;
int dowmEdge_date2 = 0;
float distance2 = 0;

int flag1 = 0; // 1-语音开 0-语音关
int flag2 = 0; // 1-语音开 0-语音关

int full_flag1 = 0; // 1-full 0-not full
int full_flag2 = 0; // 1-full 0-not full

// 24×24 图标字模（阴码，列行式，高位在前）
// 数据量：72字节
const uint8_t ICON_24x24[72] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xc0, 0xf0, 0xfc, 0x7e, 0x3f, 0x0f, 
    0x07, 0x1f, 0x7f, 0xfe, 0xf8, 0xe0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,  // 第1页（y=0~7）
    
    0x00, 0xc0, 0xf8, 0xfc, 0xfc, 0x3d, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x01, 0x01, 0x05, 0x1e, 0x7e, 0xfc, 0xf0, 0xe0, 0x80,  // 第2页（y=8~15）
    
    0x1f, 0x3f, 0x3f, 0x39, 0x38, 0x38, 0x38, 0x38, 0xf8, 0xf8, 0xf8, 0xf8, 
    0x00, 0xf8, 0xf8, 0xf8, 0x38, 0x38, 0x38, 0x38, 0x39, 0x3b, 0x3f, 0x1f   // 第3页（y=16~23）
};

// 24×24 沙漏图标字模（阴码，列行式，高位在前）
// 数据量：72字节
const uint8_t HOURGLASS_ICON[72] = {
    // 第0页（y=0~7，上半部分）
    0x00, 0x00, 0x0e, 0x1f, 0x3f, 0x7b, 0xf3, 0xe3, 0xc3, 0xc3, 0xd3, 0x0b, 
	0xc3, 0xd3, 0xc3, 0xe3, 0xf3, 0xfb, 0x7f, 0x3f, 0x1f, 0x00, 0x00, 0x00,
    
    // 第1页（y=8~15，中间部分）
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x81, 0xc3, 0xe7, 0xff, 0x00, 
	0x7f, 0xff, 0xe7, 0xc3, 0x81, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    
    // 第2页（y=16~23，下半部分）
    0x00, 0x00, 0x70, 0xf8, 0xfc, 0xde, 0xcf, 0xc7, 0xc3, 0xc1, 0xc8, 0xf0, 
	0xc0, 0xc9, 0xf3, 0xc7, 0xcf, 0xdf, 0xfe, 0xfc, 0xf8, 0x00, 0x00, 0x00
};

// 超声波检测人靠近打开垃圾桶
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
	{
		 upEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_3);
		 dowmEdge_date1 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_4);
		 distance1 = ((dowmEdge_date1 - upEdge_date1) * 0.034) / 2;
//		 OLED_ShowFloat(1, 13, distance1, 2, 1);
		 if(distance1 <= 4)
		 {
			 servo1_open(); // 超声波识别人控制垃圾桶开
			 voice_mode1 = false;
		 }
		 else
		 {
			 if(voice_mode1)
			 {
				 flag1 = 1;
			 }
			 else
			 {
				 servo1_close(); // 超声波识别人控制垃圾桶开
			 }
		 }
	}
	else if(htim == &htim8 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
	{
		 upEdge_date2 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_1);
		 dowmEdge_date2 = HAL_TIM_ReadCapturedValue(&htim8, TIM_CHANNEL_2);
		 distance2 = ((dowmEdge_date2 - upEdge_date2) * 0.034) / 2;
//		 OLED2_ShowFloat(1, 13, distance2, 2, 1);
		 if(distance2 <= 4)
		 {
			 servo2_open(); // 超声波识别人控制垃圾桶开
			 voice_mode2 = false;
		 }
		 else
		 {
			 if(voice_mode2)
			 {
				 flag2 = 1;
			 }
			 else
			 {
				 servo2_close(); // 超声波识别人控制垃圾桶开
			 }

		 }
	}
}

uint16_t adc_buffer[4];
uint32_t value1;  // 从 DMA 缓冲读取
uint32_t value2;  // 从 DMA 缓冲读取
uint32_t value3;  // 从 DMA 缓冲读取
uint32_t value4;  // 从 DMA 缓冲读取

// 烟雾报警器
// 满溢检测
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc == &hadc1)
    {
      value1 = adc_buffer[0];  // 从 DMA 缓冲读取
      value2 = adc_buffer[1];  // 从 DMA 缓冲读取
      value3 = adc_buffer[2];  // 从 DMA 缓冲读取
      value4 = adc_buffer[3];  // 从 DMA 缓冲读取
      OLED_ShowNum(3, 1, value1, 4);    // 原始值 (4 位，右对齐)        /* MQ-2 1 */
      OLED2_ShowNum(3, 1, value4, 4);   // 原始值 (4 位，右对齐)	       /* MQ-2 2 */

      if(value2 < 1000)	                             /* TCRT5000 1  */
	  {
    	  full_flag2 = 1;
    	  OLED_ShowString(4, 1, "Full    ");
	  }
      else
      {
    	  full_flag2 = 0;
    	  OLED_ShowString(4, 1, "Not Full");
      }
      if(value3 < 1000)	                             /* TCRT5000 2 */
	  {
    	  full_flag1 = 1;
		  OLED2_ShowString(4, 1, "Full    ");
	  }
      else
	  {
    	  full_flag1 = 0;
		  OLED2_ShowString(4, 1, "Not Full");
	  }

      // 烟雾报警只控制蜂鸣器，RGB灯在主循环按优先级统一控制
      if(value1 >= 1500) //厨余垃圾烟雾报警
      {
    	  Beep1_TurnOn();
      }
      if(value4 >= 1500) //干垃圾烟雾报警
	  {
		Beep2_TurnOn();
	  }
    }
}


uint8_t receive_buffer[1] = {0};
uint8_t season_modle = 2;
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &huart2)
	{
		if(receive_buffer[0] == 1)
		{
			servo1_open();
		}
		else if(receive_buffer[0] == 2)
		{
			servo1_close();
		}
		else if(receive_buffer[0] == 3)
		{
			servo2_open();
		}
		else if(receive_buffer[0] == 4)
		{
			servo2_close();
		}
		else if(receive_buffer[0] == 5)
		{
			UV_Open();   // 语音控制打开紫外灯
			UV2_Open();  // 语音控制打开紫外灯
		}
		else if(receive_buffer[0] == 6)
		{
			UV_Close();   // 语音控制关闭紫外灯
			UV2_Close();  // 语音控制关闭紫外灯
		}
		else if(receive_buffer[0] == 7)
		{
			season_modle = 1; // 春
		}
		else if(receive_buffer[0] == 8)
		{
			season_modle = 2; // 夏
		}
		else if(receive_buffer[0] == 9)
		{
			season_modle = 3; // 秋
		}
		else if(receive_buffer[0] == 10)
		{
			season_modle = 4; // 冬
		}

		HAL_UART_Transmit_DMA(&huart2, receive_buffer, 1);
		HAL_UART_Receive_DMA(&huart2, receive_buffer, 1);
	}
}

void User_Main(void)
{
    /* 函数初始化 */
    servo_init();
    OLED_Init();
    OLED2_Init();
    AHT30_Init();
    AHT302_Init();
    ultrasonic_init();
    /* 参数定义 */
    float temperature, humidity;
    float temperature2, humidity2;

    HAL_ADCEx_Calibration_Start(&hadc1);
    while(1)
    {
		HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, sizeof(adc_buffer) / sizeof(uint16_t));
		HAL_UART_Receive_DMA(&huart2, receive_buffer, 1);
		ultrasonic_task1();// 超声波1
		ultrasonic_task2();// 超声波2
		UV_Key_Scan();     // UV灯按键扫描 

		OLED_ShowString(1, 1, "Food Waste");
		OLED2_ShowString(1, 1, "Residual Waste");

		AHT30_Read(&temperature, &humidity);     // 温湿度
		OLED_ShowFloat(2, 1, temperature, 2, 2);
		OLED_ShowDegreeCelsius(2, 6);
		OLED_ShowFloat(2, 9, humidity, 2, 2);
		OLED_ShowString(2, 14, "%");
		OLED_ShowIcon24x24(104, 5, HOURGLASS_ICON);	// 显示图标在 (x=52, y=2) 屏幕中央

		OLED_ShowNum(3, 8, season_modle, 1); // 季节模式打印

		AHT302_Read(&temperature2, &humidity2);  // 温湿度
		OLED2_ShowFloat(2, 1, temperature2, 2, 2);
		OLED2_ShowDegreeCelsius(2, 6);
		OLED2_ShowFloat(2, 9, humidity2, 2, 2);
		OLED2_ShowString(2, 14, "%");
		OLED2_ShowIcon24x24(104, 5, ICON_24x24);	// 显示图标在 (x=52, y=2) 屏幕中央

		if(season_modle == 1) { // 春季模式
			// 厨余垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value1 >= 1500) {                                      //烟雾最高优先级
				Beep1_TurnOn();
				Blue_TurnOn(); Red_TurnOff(); Green_TurnOff();
			}
			else if(temperature <= 20 || temperature >= 25) {         //温度次优先级
				Beep1_TurnOn();
				Red_TurnOn(); Blue_TurnOff(); Green_TurnOff();
			}
			else if(humidity <= 50 || humidity >= 70) {               //湿度最低优先级
				Beep1_TurnOn();
				Green_TurnOn(); Blue_TurnOff(); Red_TurnOff();
			}
			else {                                                     //全部正常
				Blue_TurnOff(); Red_TurnOff(); Green_TurnOff();
				Beep1_TurnOff();
			}
			// 干垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value4 >= 1500) {                                      //烟雾最高优先级
				Beep2_TurnOn();
				Blue2_TurnOn(); Red2_TurnOff(); Green2_TurnOff();
			}
			else if(temperature2 <= 20 || temperature2 >= 25) {       //温度次优先级
				Beep2_TurnOn();
				Red2_TurnOn(); Blue2_TurnOff(); Green2_TurnOff();
			}
			else if(humidity2 <= 50 || humidity2 >= 70) {             //湿度最低优先级
				Beep2_TurnOn();
				Green2_TurnOn(); Blue2_TurnOff(); Red2_TurnOff();
			}
			else {                                                     //全部正常
				Blue2_TurnOff(); Red2_TurnOff(); Green2_TurnOff();
				Beep2_TurnOff();
			}
		}
		else if(season_modle == 2) { // 夏季模式
			// 厨余垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value1 >= 1500) {
				Beep1_TurnOn();
				Blue_TurnOn(); Red_TurnOff(); Green_TurnOff();
			}
			else if(temperature <= 25 || temperature >= 30) {
				Beep1_TurnOn();
				Red_TurnOn(); Blue_TurnOff(); Green_TurnOff();
			}
			else if(humidity <= 60 || humidity >= 80) {
				Beep1_TurnOn();
				Green_TurnOn(); Blue_TurnOff(); Red_TurnOff();
			}
			else {
				Blue_TurnOff(); Red_TurnOff(); Green_TurnOff();
				Beep1_TurnOff();
			}
			// 干垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value4 >= 1500) {
				Beep2_TurnOn();
				Blue2_TurnOn(); Red2_TurnOff(); Green2_TurnOff();
			}
			else if(temperature2 <= 25 || temperature2 >= 30) {
				Beep2_TurnOn();
				Red2_TurnOn(); Blue2_TurnOff(); Green2_TurnOff();
			}
			else if(humidity2 <= 60 || humidity2 >= 80) {
				Beep2_TurnOn();
				Green2_TurnOn(); Blue2_TurnOff(); Red2_TurnOff();
			}
			else {
				Blue2_TurnOff(); Red2_TurnOff(); Green2_TurnOff();
				Beep2_TurnOff();
			}
		}
		else if(season_modle == 3) { // 秋季模式
			// 厨余垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value1 >= 1500) {
				Beep1_TurnOn();
				Blue_TurnOn(); Red_TurnOff(); Green_TurnOff();
			}
			else if(temperature <= 15 || temperature >= 22) {
				Beep1_TurnOn();
				Red_TurnOn(); Blue_TurnOff(); Green_TurnOff();
			}
			else if(humidity <= 46 || humidity >= 65) {
				Beep1_TurnOn();
				Green_TurnOn(); Blue_TurnOff(); Red_TurnOff();
			}
			else {
				Blue_TurnOff(); Red_TurnOff(); Green_TurnOff();
				Beep1_TurnOff();
			}
			// 干垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value4 >= 1500) {
				Beep2_TurnOn();
				Blue2_TurnOn(); Red2_TurnOff(); Green2_TurnOff();
			}
			else if(temperature2 <= 15 || temperature2 >= 22) {
				Beep2_TurnOn();
				Red2_TurnOn(); Blue2_TurnOff(); Green2_TurnOff();
			}
			else if(humidity2 <= 46 || humidity2 >= 65) {
				Beep2_TurnOn();
				Green2_TurnOn(); Blue2_TurnOff(); Red2_TurnOff();
			}
			else {
				Blue2_TurnOff(); Red2_TurnOff(); Green2_TurnOff();
				Beep2_TurnOff();
			}
		}
		else if(season_modle == 4) { // 冬季模式
			// 厨余垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value1 >= 1500) {
				Beep1_TurnOn();
				Blue_TurnOn(); Red_TurnOff(); Green_TurnOff();
			}
			else if(temperature <= 5 || temperature >= 15) {
				Beep1_TurnOn();
				Red_TurnOn(); Blue_TurnOff(); Green_TurnOff();
			}
			else if(humidity <= 30 || humidity >= 50) {
				Beep1_TurnOn();
				Green_TurnOn(); Blue_TurnOff(); Red_TurnOff();
			}
			else {
				Blue_TurnOff(); Red_TurnOff(); Green_TurnOff();
				Beep1_TurnOff();
			}
			// 干垃圾 RGB 优先级：烟雾蓝 > 温度红 > 湿度绿
			if(value4 >= 1500) {
				Beep2_TurnOn();
				Blue2_TurnOn(); Red2_TurnOff(); Green2_TurnOff();
			}
			else if(temperature2 <= 5 || temperature2 >= 15) {
				Beep2_TurnOn();
				Red2_TurnOn(); Blue2_TurnOff(); Green2_TurnOff();
			}
			else if(humidity2 <= 30 || humidity2 >= 50) {
				Beep2_TurnOn();
				Green2_TurnOn(); Blue2_TurnOff(); Red2_TurnOff();
			}
			else {
				Blue2_TurnOff(); Red2_TurnOff(); Green2_TurnOff();
				Beep2_TurnOff();
			}
		}
    }
}