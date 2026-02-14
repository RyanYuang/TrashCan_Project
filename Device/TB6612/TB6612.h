#ifndef _TB6612_H
#define _TB6612_H

#include "main.h"

// Define the TB6612 motor driver pins
#define TB6612_AIN1_PORT GPIOB
#define TB6612_AIN1_GPIO GPIO_PIN_4
#define TB6612_AIN2_PORT GPIOB
#define TB6612_AIN2_GPIO GPIO_PIN_5

#define TB6612_BIN1_PORT GPIOD
#define TB6612_BIN1_GPIO GPIO_PIN_2
#define TB6612_BIN2_PORT GPIOC
#define TB6612_BIN2_GPIO GPIO_PIN_2

#define TB6612_STBY_PORT GPIOB
#define TB6612_STBY_GPIO GPIO_PIN_3


// Define the PWM Timer and Channel (根据实际硬件连接修改)
#define TB6612_PWMA_TIMER htim4
#define TB6612_PWMA_CHANNEL TIM_CHANNEL_2
#define TB6612_PWMB_TIMER htim4
#define TB6612_PWMB_CHANNEL TIM_CHANNEL_3

// 定义 TB6612 的方向
#define TB6612A_FORWARD 1   
#define TB6612B_FORWARD 0

// 定义TB6612的通道编号
#define TB6612_PWMA 0
#define TB6612_PWMB 1

// 定义速度范围
/* 不得更改 */
#define TB6612_MAX_SPEED 700
#define TB6612_MIN_SPEED -700

// Function prototypes
void TB6612_Init(void);
void TB6612_SetSpeed(int l_speed, int r_speed);
void TB6612_Stop(void);
static void TB6612_SetDirection(uint8_t channel, int speed);

#endif
