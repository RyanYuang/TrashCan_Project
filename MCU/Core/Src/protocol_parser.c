/**
  ******************************************************************************
  * @file           : protocol_parser.c
  * @brief          : 串口协议解析模块实现
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  * @note           : 阈值帧为 #O<trig>,<safe>,G<low>,<high>，见 protocol_parser.h
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "protocol_parser.h"
#include "uart_protocol.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

// 调试日志开关（注释掉这行可关闭调试输出）
//#define PROTOCOL_DEBUG_ENABLE

#ifdef PROTOCOL_DEBUG_ENABLE
    #define DEBUG_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...)
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 最新接收到的控制指令
static ControlCommand_t g_LastCommand = CMD_INVALID;

// 回调函数指针
static CommandCallback_t g_CommandCallback = NULL;
static ThresholdCallback_t g_ThresholdCallback = NULL;

/* Private function prototypes -----------------------------------------------*/
static ParseResult_t Parse_ControlCommand(const char *frame);
static ParseResult_t Parse_ThresholdConfig(const char *frame);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 解析控制指令帧：@<指令码>\r\n
 * @param frame 协议帧字符串
 * @retval 解析结果
 */
static ParseResult_t Parse_ControlCommand(const char *frame)
{
    if (frame == NULL || frame[0] != '@') {
        return PARSE_ERROR;
    }
    
    // 跳过'@'字符
    const char *cmdStr = &frame[1];
    
    // 跳过前导空格
    while (*cmdStr == ' ') {
        cmdStr++;
    }
    
    // 解析指令码
    if (!isdigit(*cmdStr)) {
        return PARSE_ERROR;
    }
    
    int cmdValue = atoi(cmdStr);
    printf("cmdValue:%d\r\n",cmdValue);
    
    // 验证指令码范围（0～8 运动/速度；9～10 工作模式）
    if (cmdValue < 0 || cmdValue > 10) {
        g_LastCommand = CMD_INVALID;
        return PARSE_ERROR;
    }
    
    g_LastCommand = (ControlCommand_t)cmdValue;
    
    // 调用回调函数
    if (g_CommandCallback != NULL) {
        g_CommandCallback(g_LastCommand);
    }
    
    return PARSE_OK;
}

/**
 * @brief 解析阈值配置帧：#O<trig_cm>,<safe_cm>,G<low_ppm>,<high_ppm>\r\n
 * @note trig：小于该距离(cm)视为过近；safe：大于该距离(cm)视为可恢复；须 0<trig<safe<=500。
 *       气体须 low<high；low==0 表示仅使用上限报警（由应用层 enable_low_alarm=false 体现）。
 */
static ParseResult_t Parse_ThresholdConfig(const char *frame)
{
    static const char kErrParse[] = "#ERR:Parse failed\r\n";
    static const char kErrParam[] = "#ERR:Invalid params\r\n";
    static const char kOk[] = "#OK:Config synced\r\n";

    if (frame == NULL || frame[0] != '#') {
        DEBUG_LOG("[ThresholdParse] invalid frame\r\n");
        return PARSE_ERROR;
    }

    DEBUG_LOG("[ThresholdParse] raw: %s\r\n", frame);

    unsigned trig = 0;
    unsigned safe = 0;
    float glo = 0.0f;
    float ghi = 0.0f;
    int n = sscanf(frame, "#O%u,%u,G%f,%f", &trig, &safe, &glo, &ghi);
    if (n != 4) {
        HAL_UART_Transmit(&huart2, (uint8_t *)kErrParse, (uint16_t)(sizeof(kErrParse) - 1U), 100);
        return PARSE_ERROR;
    }

    if (trig == 0U || safe == 0U || trig >= safe || trig > 500U || safe > 500U) {
        HAL_UART_Transmit(&huart2, (uint8_t *)kErrParam, (uint16_t)(sizeof(kErrParam) - 1U), 100);
        return PARSE_ERROR;
    }

    if (!(glo < ghi)) {
        HAL_UART_Transmit(&huart2, (uint8_t *)kErrParam, (uint16_t)(sizeof(kErrParam) - 1U), 100);
        return PARSE_ERROR;
    }

    ProtocolEnvLimits_t lim;
    lim.obstacle_trig_cm = (uint16_t)trig;
    lim.obstacle_safe_cm = (uint16_t)safe;
    lim.gas_low_ppm = glo;
    lim.gas_high_ppm = ghi;

    if (g_ThresholdCallback != NULL) {
        g_ThresholdCallback(&lim);
    }

    HAL_UART_Transmit(&huart2, (uint8_t *)kOk, (uint16_t)(sizeof(kOk) - 1U), 100);
    return PARSE_OK;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化协议解析模块
 */
void Protocol_Parser_Init(void)
{
    g_LastCommand = CMD_INVALID;
    g_CommandCallback = NULL;
    g_ThresholdCallback = NULL;
}

/**
 * @brief 注册控制指令回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterCommandCallback(CommandCallback_t callback)
{
    g_CommandCallback = callback;
}

/**
 * @brief 注册阈值配置回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterThresholdCallback(ThresholdCallback_t callback)
{
    g_ThresholdCallback = callback;
}

/**
 * @brief 解析接收到的协议帧
 * @param frame 协议帧字符串（以\0结尾）
 * @retval 解析结果
 */
ParseResult_t Protocol_Parse(const char *frame)
{
    if (frame == NULL || frame[0] == '\0') {
        return PARSE_ERROR;
    }
    
    // 根据首字符分发到对应的解析函数
    switch (frame[0]) {
        case '@':
            // 控制指令帧
            return Parse_ControlCommand(frame);
            
        case '#':
            // 阈值配置帧
        	DEBUG_LOG("[Protocol] Threshold config frame\r\n");
            return Parse_ThresholdConfig(frame);
            
        default:
            // 未知帧类型
            return PARSE_UNKNOWN_TYPE;
    }
}

/**
 * @brief 获取最新的控制指令
 * @retval 控制指令
 */
ControlCommand_t Protocol_GetLastCommand(void)
{
    return g_LastCommand;
}

/**
 * @brief 清除最新的控制指令（标记为已处理）
 */
void Protocol_ClearLastCommand(void)
{
    g_LastCommand = CMD_INVALID;
}

/**
 * @brief 获取控制指令的字符串描述
 * @param cmd 控制指令
 * @return 指令字符串描述
 */
const char* Protocol_GetCommandString(ControlCommand_t cmd)
{
    switch (cmd) {
        case CMD_STOP:
            return "Stop";
        case CMD_FORWARD:
            return "Forward";
        case CMD_BACKWARD:
            return "Backward";
        case CMD_TURN_LEFT:
            return "Turn Left";
        case CMD_TURN_RIGHT:
            return "Turn Right";
        case CMD_SPEED_25:
            return "Speed 25%";
        case CMD_SPEED_50:
            return "Speed 50%";
        case CMD_SPEED_75:
            return "Speed 75%";
        case CMD_SPEED_100:
            return "Speed 100%";
        case CMD_MODE_MANUAL:
            return "Mode Manual";
        case CMD_MODE_AUTO_TRACK:
            return "Mode Auto Track";
        default:
            return "Invalid";
    }
}
