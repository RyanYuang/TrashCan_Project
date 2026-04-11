/**
  ******************************************************************************
  * @file           : uart_protocol.h
  * @brief          : UART串口协议接收模块头文件
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  * @description
  * 该模块实现基于USART中断的行缓冲接收机制
  * 支持以\r\n为结束符的文本协议帧接收
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
 * @brief 发送传感器数据（上行帧）
 * @param huart UART句柄指针
 * @param temp 温度 (°C)
 * @param hum 湿度 (%)
 * @param co CO浓度 (ppm)
 * @param light 光照强度 (lux)
 * @retval HAL状态
 */
HAL_StatusTypeDef UART_Protocol_SendSensorData(UART_HandleTypeDef *huart, 
                                                float temp, float hum, 
                                                float co, float light);

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
