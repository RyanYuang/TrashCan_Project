/**
  ******************************************************************************
  * @file           : main_template.c
  * @brief          : 主函数集成模板（复制到 main.c 的 USER CODE 区域）
  * @author         : Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  * 使用说明：
  * 1. 复制以下代码到 main.c 对应的 USER CODE 区域
  * 2. 替换传感器读取函数（Read_XXX）为你的实际函数
  * 3. 替换电机控制函数（Control_XXX）为你的实际函数
  ******************************************************************************
  */

// ============================================================
// 步骤1：在 main.c 开头添加（USER CODE BEGIN Includes）
// ============================================================
/*
#include "uart_protocol.h"
#include "protocol_parser.h"
#include "threshold_config.h"
*/

// ============================================================
// 步骤2：在全局变量区域添加（USER CODE BEGIN PV）
// ============================================================
/*
// 电机控制状态
uint8_t g_MotorSpeed = 50;  // 当前速度（0-100%）
uint8_t g_MotorState = 0;   // 当前状态：0=停止, 1=前进, 2=后退, 3=左转, 4=右转
*/

// ============================================================
// 步骤3：在函数声明区域添加（USER CODE BEGIN PFP）
// ============================================================
/*
void OnCommandReceived(ControlCommand_t cmd);
void UpdateMotorControl(void);
*/

// ============================================================
// 步骤4：在 main() 初始化区域添加（USER CODE BEGIN 2）
// ============================================================
/*
// 初始化阈值配置（使用默认值）
ThresholdConfig_Init(NULL);

// 初始化协议解析器
Protocol_Parser_Init();

// 注册指令回调函数（收到指令时自动调用）
Protocol_Parser_RegisterCommandCallback(OnCommandReceived);

// 初始化串口协议（假设使用 USART1，根据实际情况修改）
UART_Protocol_Init(&huart1);
*/

// ============================================================
// 步骤5：在 main() 主循环添加（USER CODE BEGIN WHILE）
// ============================================================
/*
// --------------- 1. 处理接收到的串口指令 ---------------
if (UART_Protocol_IsFrameReady()) {
    char rxFrame[256];
    uint16_t len = UART_Protocol_GetFrame(rxFrame, sizeof(rxFrame));
    
    if (len > 0) {
        // 解析协议帧（@指令 或 #阈值配置）
        ParseResult_t result = Protocol_Parse(rxFrame);
        
        // 可选：打印解析结果（调试用）
        // if (result != PARSE_OK) {
        //     printf("Parse error: %s\r\n", rxFrame);
        // }
    }
}

// --------------- 2. 定时发送传感器数据 ---------------
static uint32_t lastSendTime = 0;
if (HAL_GetTick() - lastSendTime >= 500) {  // 每500ms发送一次
    lastSendTime = HAL_GetTick();
    
    // TODO: 替换为你的真实传感器读取函数
    float temp = 25.6;     // Read_AHT30_Temperature();
    float hum = 60.3;      // Read_AHT30_Humidity();
    float co = 0.5;        // Read_MQ2_CO();
    float light = 450.0;   // Read_GY302_Light();
    
    // 发送到上位机（格式：@temp,hum,co,light\r\n）
    UART_Protocol_SendSensorData(&huart1, temp, hum, co, light);
    
    // 可选：检查是否超过阈值
    SensorData_t data = {temp, hum, co, light};
    AlertLevel_t alert = ThresholdConfig_CheckAlert(&data);
    if (alert == ALERT_DANGER) {
        // TODO: 打开蜂鸣器或红灯
        // HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
    }
}

// --------------- 3. 更新电机控制 ---------------
UpdateMotorControl();
*/

// ============================================================
// 步骤6：在函数定义区域添加（USER CODE BEGIN 4）
// ============================================================
/*
// 指令回调函数：收到控制指令时自动调用
void OnCommandReceived(ControlCommand_t cmd)
{
    switch(cmd) {
        case CMD_STOP:
            g_MotorState = 0;
            break;
            
        case CMD_FORWARD:
            g_MotorState = 1;
            break;
            
        case CMD_BACKWARD:
            g_MotorState = 2;
            break;
            
        case CMD_TURN_LEFT:
            g_MotorState = 3;
            break;
            
        case CMD_TURN_RIGHT:
            g_MotorState = 4;
            break;
            
        case CMD_SPEED_25:
            g_MotorSpeed = 25;
            break;
            
        case CMD_SPEED_50:
            g_MotorSpeed = 50;
            break;
            
        case CMD_SPEED_75:
            g_MotorSpeed = 75;
            break;
            
        case CMD_SPEED_100:
            g_MotorSpeed = 100;
            break;
            
        default:
            break;
    }
}

// 电机控制函数：根据状态控制电机
void UpdateMotorControl(void)
{
    // TODO: 根据 g_MotorState 和 g_MotorSpeed 控制电机
    
    switch(g_MotorState) {
        case 0:  // 停止
            // 示例：关闭电机使能
            // HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, GPIO_PIN_RESET);
            break;
            
        case 1:  // 前进
            // 示例：设置前进方向 + PWM速度
            // HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, GPIO_PIN_SET);
            // __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, g_MotorSpeed);
            break;
            
        case 2:  // 后退
            // 设置后退方向 + PWM速度
            break;
            
        case 3:  // 左转
            // 左轮停止，右轮转动
            break;
            
        case 4:  // 右转
            // 右轮停止，左轮转动
            break;
    }
}
*/

// ============================================================
// 步骤7：在 stm32f1xx_it.c 添加中断回调
// ============================================================
/*
// 在文件开头添加
#include "uart_protocol.h"

// 在文件末尾添加或找到已有的回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 调用串口协议接收处理
    UART_Protocol_RxCallback(huart);
}
*/

// ============================================================
// 完成！编译运行即可
// ============================================================
