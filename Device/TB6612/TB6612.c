#include "TB6612.h"
#include "tim.h"  // 包含定时器头文件

/**
 * @brief 初始化TB6612电机驱动
 */
void TB6612_Init(void)
{
    // 初始化GPIO引脚
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    // 使能GPIOA时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();
    
    // 配置AIN1, AIN2, BIN1, BIN2引脚
    GPIO_InitStruct.Pin = TB6612_AIN1_GPIO | TB6612_AIN2_GPIO | 
                          TB6612_BIN1_GPIO | TB6612_BIN2_GPIO;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;  // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 初始化所有引脚为低电平
    HAL_GPIO_WritePin(TB6612_AIN1_PORT, TB6612_AIN1_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_AIN2_PORT, TB6612_AIN2_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN1_PORT, TB6612_BIN1_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN2_PORT, TB6612_BIN2_GPIO, GPIO_PIN_RESET);
    
    // 启动PWM定时器
    HAL_TIM_PWM_Start(&TB6612_PWMA_TIMER, TB6612_PWMA_CHANNEL);
    HAL_TIM_PWM_Start(&TB6612_PWMB_TIMER, TB6612_PWMB_CHANNEL);
}

/**
 * @brief 设置电机方向
 * @param channel 电机通道 (TB6612_PWMA 或 TB6612_PWMB)
 * @param speed 速度值，正值为正转，负值为反转
 */
static void TB6612_SetDirection(uint8_t channel, int speed)
{
    if (channel == TB6612_PWMA) {
        if (speed >= 0) {
            // 正转
            HAL_GPIO_WritePin(TB6612_AIN1_PORT, TB6612_AIN1_GPIO, GPIO_PIN_SET);
            HAL_GPIO_WritePin(TB6612_AIN2_PORT, TB6612_AIN2_GPIO, GPIO_PIN_RESET);
        } else {
            // 反转
            HAL_GPIO_WritePin(TB6612_AIN1_PORT, TB6612_AIN1_GPIO, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(TB6612_AIN2_PORT, TB6612_AIN2_GPIO, GPIO_PIN_SET);
        }
    } else if (channel == TB6612_PWMB) {
        if (speed >= 0) {
            // 正转
            HAL_GPIO_WritePin(TB6612_BIN1_PORT, TB6612_BIN1_GPIO, GPIO_PIN_SET);
            HAL_GPIO_WritePin(TB6612_BIN2_PORT, TB6612_BIN2_GPIO, GPIO_PIN_RESET);
        } else {
            // 反转
            HAL_GPIO_WritePin(TB6612_BIN1_PORT, TB6612_BIN1_GPIO, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(TB6612_BIN2_PORT, TB6612_BIN2_GPIO, GPIO_PIN_SET);
        }
    }
}

/**
 * @brief 设置左右电机速度
 * @param l_speed 左轮速度 (-1000 ~ 1000)
 * @param r_speed 右轮速度 (-1000 ~ 1000)
 */
void TB6612_SetSpeed(int l_speed, int r_speed)
{
    // 限制速度范围
    if (l_speed > TB6612_MAX_SPEED) l_speed = TB6612_MAX_SPEED;
    if (l_speed < TB6612_MIN_SPEED) l_speed = TB6612_MIN_SPEED;
    if (r_speed > TB6612_MAX_SPEED) r_speed = TB6612_MAX_SPEED;
    if (r_speed < TB6612_MIN_SPEED) r_speed = TB6612_MIN_SPEED;
    

    // 根据需要调整方向
    #if TB6612A_FORWARD
        // 不需要反转
    #else
        l_speed = -l_speed;
    #endif
	// 设置左轮方向和速度
	TB6612_SetDirection(TB6612_PWMA, l_speed);

    // 根据需要调整方向
    #if TB6612B_FORWARD
        // 不需要反转
    #else
        r_speed = -r_speed;
    #endif
	// 设置右轮方向和速度
	TB6612_SetDirection(TB6612_PWMB, r_speed);
    // 设置PWM占空比 (使用绝对值)
    __HAL_TIM_SET_COMPARE(&TB6612_PWMA_TIMER, TB6612_PWMA_CHANNEL, abs(l_speed));
    __HAL_TIM_SET_COMPARE(&TB6612_PWMB_TIMER, TB6612_PWMB_CHANNEL, abs(r_speed));
}

/**
 * @brief 停止所有电机
 */
void TB6612_Stop(void)
{
    // 设置所有控制引脚为低电平
    HAL_GPIO_WritePin(TB6612_AIN1_PORT, TB6612_AIN1_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_AIN2_PORT, TB6612_AIN2_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN1_PORT, TB6612_BIN1_GPIO, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(TB6612_BIN2_PORT, TB6612_BIN2_GPIO, GPIO_PIN_RESET);
    
    // 设置PWM占空比为0
    __HAL_TIM_SET_COMPARE(&TB6612_PWMA_TIMER, TB6612_PWMA_CHANNEL, 0);
    __HAL_TIM_SET_COMPARE(&TB6612_PWMB_TIMER, TB6612_PWMB_CHANNEL, 0);
}
