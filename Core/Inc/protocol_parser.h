/**
  ******************************************************************************
  * @file           : protocol_parser.h
  * @brief          : 串口协议解析模块头文件
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  * @description
  * 该模块负责解析来自上位机的控制指令和阈值配置帧
  * 支持两种下行帧：@<指令码>\r\n 和 #T...\r\n
  ******************************************************************************
  */

#ifndef __PROTOCOL_PARSER_H
#define __PROTOCOL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "threshold_config.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 控制指令枚举
 */
typedef enum {
    CMD_STOP = 0,         // 停止
    CMD_FORWARD = 1,      // 前进
    CMD_BACKWARD = 2,     // 后退
    CMD_TURN_LEFT = 3,    // 左转
    CMD_TURN_RIGHT = 4,   // 右转
    CMD_SPEED_25 = 5,     // 速度25%
    CMD_SPEED_50 = 6,     // 速度50%
    CMD_SPEED_75 = 7,     // 速度75%
    CMD_SPEED_100 = 8,    // 速度100%
    CMD_PATROL_START = 9,    // 启动巡逻
    CMD_PATROL_STOP = 10,    // 停止巡逻
    CMD_INVALID = 0xFF    // 无效指令
} ControlCommand_t;

/**
 * @brief 协议解析结果
 */
typedef enum {
    PARSE_OK = 0,         // 解析成功
    PARSE_ERROR,          // 解析失败
    PARSE_UNKNOWN_TYPE    // 未知帧类型
} ParseResult_t;

/**
 * @brief 指令回调函数类型定义
 * @param cmd 控制指令
 */
typedef void (*CommandCallback_t)(ControlCommand_t cmd);

/**
 * @brief 阈值配置回调函数类型定义
 * @param config 新的阈值配置
 */
typedef void (*ThresholdCallback_t)(const ThresholdConfig_t *config);

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 初始化协议解析模块
 */
void Protocol_Parser_Init(void);

/**
 * @brief 注册控制指令回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterCommandCallback(CommandCallback_t callback);

/**
 * @brief 注册阈值配置回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterThresholdCallback(ThresholdCallback_t callback);

/**
 * @brief 解析接收到的协议帧
 * @param frame 协议帧字符串（以\0结尾）
 * @retval 解析结果
 */
ParseResult_t Protocol_Parse(const char *frame);

/**
 * @brief 获取最新的控制指令
 * @retval 控制指令
 */
ControlCommand_t Protocol_GetLastCommand(void);

/**
 * @brief 清除最新的控制指令（标记为已处理）
 */
void Protocol_ClearLastCommand(void);

/**
 * @brief 获取控制指令的字符串描述
 * @param cmd 控制指令
 * @return 指令字符串描述
 */
const char* Protocol_GetCommandString(ControlCommand_t cmd);

#ifdef __cplusplus
}
#endif

#endif /* __PROTOCOL_PARSER_H */
