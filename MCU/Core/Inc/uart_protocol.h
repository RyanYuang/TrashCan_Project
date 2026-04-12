/**
  ******************************************************************************
  * @file           : uart_protocol.h
  * @brief          : UART串口协议接收模块头文件
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  * @description
  * 该模块实现基于 USART 中断的行缓冲接收；支持以 \\n 结束的文本下行帧解析。
  * 上行提供 $STS: 下位机状态帧发送接口（与下行 @ / # 区分）。
  ******************************************************************************
  */

#ifndef __UART_PROTOCOL_H
#define __UART_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief UART接收状态
 */
typedef enum {
    UART_RX_IDLE = 0,     // 空闲
    UART_RX_RECEIVING,    // 接收中
    UART_RX_COMPLETE      // 接收完成（检测到\n）
} UARTRxState_t;

/**
 * @brief UART行缓冲结构体
 */
typedef struct {
    uint8_t buffer[256];  // 接收缓冲区
    uint16_t index;       // 当前写入位置
    UARTRxState_t state;  // 接收状态
    uint8_t rxByte;       // 单字节接收缓冲（用于中断）
} UARTLineBuffer_t;

/* Exported constants --------------------------------------------------------*/

#define UART_BUFFER_SIZE  256

/** 无障碍物（距离字段无效，固定填 0） */
#define UART_OBS_NONE     0u
/** 有障碍物：距离字段为最近障碍物距离 (cm) */
#define UART_OBS_NEAR     1u

/** 无警报 */
#define UART_ALM_NONE     0u
/** 障碍相关警报 */
#define UART_ALM_OBST     1u
/** 气体相关警报 */
#define UART_ALM_GAS      2u
/** 障碍 + 气体同时满足告警条件 */
#define UART_ALM_BOTH     3u

/**
 * @brief 下位机状态上行帧载荷（经 USART 周期上报）
 */
typedef struct {
    float gas_ppm;           /**< 气体浓度 (ppm) */
    uint8_t obs_flag;        /**< UART_OBS_NONE / UART_OBS_NEAR */
    uint16_t obs_cm;         /**< obs_flag==NEAR 时为距离(cm)；否则为 0 */
    uint8_t alarm;         /**< UART_ALM_* */
    uint8_t car_state;       /**< 小车状态枚举数值，与 EnvCar_State_Enum 一致 */
} UART_StatusUplink_t;

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 初始化UART协议接收模块
 * @param huart UART句柄指针（例如 &huart1）
 * @retval HAL状态
 */
HAL_StatusTypeDef UART_Protocol_Init(UART_HandleTypeDef *huart);

/**
 * @brief 启动UART中断接收
 * @param huart UART句柄指针
 * @retval HAL状态
 */
HAL_StatusTypeDef UART_Protocol_StartReceive(UART_HandleTypeDef *huart);

/**
 * @brief 检查是否有完整帧接收完毕
 * @retval true: 有完整帧, false: 无完整帧
 */
bool UART_Protocol_IsFrameReady(void);

/**
 * @brief 获取接收到的完整帧数据
 * @param outBuffer 输出缓冲区（用户提供）
 * @param maxLen 输出缓冲区最大长度
 * @retval 实际复制的字节数（0表示无数据）
 */
uint16_t UART_Protocol_GetFrame(char *outBuffer, uint16_t maxLen);

/**
 * @brief 发送下位机状态帧（上行，与下行 @/# 不冲突，前缀为 $）
 * @note 文本格式: $STS:<gas>,<obs>,<cm>,<alarm>,<car>\\r\\n
 */
HAL_StatusTypeDef UART_Protocol_SendStatusFrame(UART_HandleTypeDef *huart,
                                                const UART_StatusUplink_t *st);

/**
 * @brief UART接收完成中断回调（需在stm32f1xx_it.c中调用）
 * @param huart UART句柄指针
 * @note 该函数会在HAL_UART_RxCpltCallback中被调用
 */
void UART_Protocol_RxCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __UART_PROTOCOL_H */
