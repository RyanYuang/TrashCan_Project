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

/* UV消毒状态机（枚举定义在 User_Main.h）*/

#define UV_TOTAL_MS  (30UL * 60UL * 1000UL)  // 30分钟（毫秒）

UV_State_t uv_disinfect_state = UV_IDLE;  // 全局变量，供外部访问
static uint32_t   uv_remaining       = UV_TOTAL_MS;  // 剩余消毒时长(ms)
static uint32_t   uv_tick            = 0;            // 本段计时起点
static uint8_t    uv_alarm_active    = 0;            // 报警标志（由主循环更新）

/* 启动消毒：只有从IDLE启动时才重置时间，从暂停续时则保留 */
void UV_Disinfect_Start(void)
{
    if(uv_disinfect_state == UV_IDLE)
    {
        uv_remaining = UV_TOTAL_MS;
    }
    uv_tick            = HAL_GetTick();
    UV_Open();
    UV2_Open();
    uv_disinfect_state = UV_RUNNING;
}

/* 手动停止消毒：记录剩余时长，关灯，进入手动暂停（不会自动续时） */
void UV_Disinfect_Stop(void)
{
    if(uv_disinfect_state == UV_RUNNING)
    {
        uint32_t elapsed = HAL_GetTick() - uv_tick;
        uv_remaining = (elapsed >= uv_remaining) ? 0 : (uv_remaining - elapsed);
    }
    UV_Close();
    UV2_Close();
    uv_disinfect_state = UV_MANUAL_PAUSED;  // 手动暂停，不自动续时
}

/* 消毒任务：在主循环中轮询调用 */
static void UV_Disinfect_Task(void)
{
    uint32_t now = HAL_GetTick();
    // 需要暂停的条件：任意桶盖开 OR 有报警（烟雾/温湿度/满桶）
    uint8_t should_pause = (servo1_state || servo2_state || uv_alarm_active);

    switch(uv_disinfect_state)
    {
        case UV_IDLE:
        case UV_MANUAL_PAUSED:
            // 空闲或手动暂停：不自动操作，等待按键/语音触发
            break;

        case UV_RUNNING:
            if(should_pause)
            {
                // 需要暂停：记录剩余时长，关灯
                uint32_t elapsed = now - uv_tick;
                uv_remaining = (elapsed >= uv_remaining) ? 0 : (uv_remaining - elapsed);
                UV_Close();
                UV2_Close();
                uv_disinfect_state = UV_PAUSED;
            }
            else if((now - uv_tick) >= uv_remaining)
            {
                // 消毒完成
                UV_Close();
                UV2_Close();
                uv_remaining       = UV_TOTAL_MS;
                uv_disinfect_state = UV_IDLE;
            }
            break;

        case UV_PAUSED:
            if(!should_pause)
            {
                // 桶盖关闭且无报警：自动续时，开灯
                uv_tick            = HAL_GetTick();
                UV_Open();
                UV2_Open();
                uv_disinfect_state = UV_RUNNING;
            }
            break;
    }
}

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
			UV_Disinfect_Start();  // 语音控制启动消毒（30分钟计时）
		}
		else if(receive_buffer[0] == 6)
		{
			UV_Disinfect_Stop();   // 语音控制停止消毒
		}
		else if(receive_buffer[0] == 0x07)
		{
			season_modle = 1; // 春
		}
		else if(receive_buffer[0] == 0x08)
		{
			season_modle = 2; // 夏
		}
		else if(receive_buffer[0] == 0x09)
		{
			season_modle = 3; // 秋
		}
		else if(receive_buffer[0] == 0x10)
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
		UV_Disinfect_Task(); // UV消毒计时状态机

		// OLED2 第3行显示剩余消毒时间
		if(uv_disinfect_state == UV_IDLE)
		{
			OLED2_ShowString(3, 6, "--:--");
		}
		else
		{
			// 计算当前实际剩余时长（RUNNING状态需减去已过时间）
			uint32_t display_ms = uv_remaining;
			if(uv_disinfect_state == UV_RUNNING)
			{
				uint32_t elapsed = HAL_GetTick() - uv_tick;
				display_ms = (elapsed >= uv_remaining) ? 0 : (uv_remaining - elapsed);
			}
			uint32_t total_sec = display_ms / 1000;
			uint8_t  min       = total_sec / 60;
			uint8_t  sec       = total_sec % 60;
			OLED2_ShowNum(3, 6, min, 2);
			OLED2_ShowString(3, 8, ":");
			OLED2_ShowNum(3, 9, sec, 2);
		}

		OLED_ShowString(1, 1, "Food Waste");
		OLED2_ShowString(1, 1, "Residual Waste");

		AHT30_Read(&temperature, &humidity);     // 温湿度
		OLED_ShowFloat(2, 1, temperature, 2, 2);
		OLED_ShowDegreeCelsius(2, 6);
		OLED_ShowFloat(2, 9, humidity, 2, 2);
		OLED_ShowString(2, 14, "%");
		OLED_ShowIcon24x24(104, 5, HOURGLASS_ICON);	// 显示图标在 (x=52, y=2) 屏幕中央

		OLED_ShowString(3, 6, "Mode:");	
		OLED_ShowNum(3, 11, season_modle, 1); // 季节模式打印

		AHT302_Read(&temperature2, &humidity2);  // 温湿度
		OLED2_ShowFloat(2, 1, temperature2, 2, 2);
		OLED2_ShowDegreeCelsius(2, 6);
		OLED2_ShowFloat(2, 9, humidity2, 2, 2);
		OLED2_ShowString(2, 14, "%");
		OLED2_ShowIcon24x24(104, 5, ICON_24x24);	// 显示图标在 (x=52, y=2) 屏幕中央

		// 更新UV消毒报警标志（优先级最低，有报警或满桶时暂停消毒）
		uint8_t smoke_alarm    = (value1 >= 1500 || value4 >= 1500);
		uint8_t full_alarm     = (full_flag1 || full_flag2);
		uint8_t temp_humi_alarm = 0;
		if(season_modle == 1)
			temp_humi_alarm = (temperature<=20||temperature>=25||humidity<=50||humidity>=70||
			                   temperature2<=20||temperature2>=25||humidity2<=50||humidity2>=70);
		else if(season_modle == 2)
			temp_humi_alarm = (temperature<=25||temperature>=30||humidity<=60||humidity>=80||
			                   temperature2<=25||temperature2>=30||humidity2<=60||humidity2>=80);
		else if(season_modle == 3)
			temp_humi_alarm = (temperature<=15||temperature>=22||humidity<=46||humidity>=65||
			                   temperature2<=15||temperature2>=22||humidity2<=46||humidity2>=65);
		else if(season_modle == 4)
			temp_humi_alarm = (temperature<=5||temperature>=15||humidity<=30||humidity>=50||
			                   temperature2<=5||temperature2>=15||humidity2<=30||humidity2>=50);
		uv_alarm_active = (smoke_alarm || full_alarm || temp_humi_alarm);

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