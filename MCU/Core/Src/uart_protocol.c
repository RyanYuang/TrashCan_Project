/**
  ******************************************************************************
  * @file           : uart_protocol.c
  * @brief          : UART串口协议接收模块实现
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "uart_protocol.h"
#include <stdio.h>
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 行缓冲区
static UARTLineBuffer_t g_RxLineBuffer = {0};

// UART句柄（保存用于重启接收）
static UART_HandleTypeDef *g_pUartHandle = NULL;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化UART协议接收模块
 * @param huart UART句柄指针（例如 &huart1）
 * @retval HAL状态
 */
HAL_StatusTypeDef UART_Protocol_Init(UART_HandleTypeDef *huart)
{
    if (huart == NULL) {
        return HAL_ERROR;
    }
    
    // 保存UART句柄
    g_pUartHandle = huart;
    
    // 初始化行缓冲区
    memset(&g_RxLineBuffer, 0, sizeof(UARTLineBuffer_t));
    g_RxLineBuffer.state = UART_RX_IDLE;
    g_RxLineBuffer.index = 0;
    
    // 启动中断接收
    return UART_Protocol_StartReceive(huart);
}

/**
 * @brief 启动UART中断接收
 * @param huart UART句柄指针
 * @retval HAL状态
 */
HAL_StatusTypeDef UART_Protocol_StartReceive(UART_HandleTypeDef *huart)
{
    if (huart == NULL) {
        return HAL_ERROR;
    }
    
    // 启动单字节中断接收
    return HAL_UART_Receive_IT(huart, &g_RxLineBuffer.rxByte, 1);
}

/**
 * @brief 检查是否有完整帧接收完毕
 * @retval true: 有完整帧, false: 无完整帧
 */
bool UART_Protocol_IsFrameReady(void)
{
    return (g_RxLineBuffer.state == UART_RX_COMPLETE);
}

/**
 * @brief 获取接收到的完整帧数据
 * @param outBuffer 输出缓冲区（用户提供）
 * @param maxLen 输出缓冲区最大长度
 * @retval 实际复制的字节数（0表示无数据）
 */
uint16_t UART_Protocol_GetFrame(char *outBuffer, uint16_t maxLen)
{
    if (outBuffer == NULL || maxLen == 0) {
        return 0;
    }
    
    // 检查是否有完整帧
    if (g_RxLineBuffer.state != UART_RX_COMPLETE) {
        return 0;
    }
    
    // 计算实际复制长度（不超过缓冲区大小）
    uint16_t copyLen = (g_RxLineBuffer.index < maxLen) ? 
                       g_RxLineBuffer.index : (maxLen - 1);
    
    // 复制数据
    memcpy(outBuffer, g_RxLineBuffer.buffer, copyLen);
    outBuffer[copyLen] = '\0';  // 添加字符串结束符
    
    // 重置缓冲区状态
    g_RxLineBuffer.index = 0;
    g_RxLineBuffer.state = UART_RX_IDLE;
    
    return copyLen;
}

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
                                                float co, float light)
{
    char txBuffer[64];
    int len;
    
    if (huart == NULL) {
        return HAL_ERROR;
    }
    
    // 格式化为协议帧：@<温度>,<湿度>,<CO>,<光照>\r\n
    len = snprintf(txBuffer, sizeof(txBuffer), 
                   "@%.1f,%.1f,%.1f,%.0f\r\n", 
                   temp, hum, co, light);
    
    if (len > 0 && len < sizeof(txBuffer)) {
        return HAL_UART_Transmit(huart, (uint8_t*)txBuffer, len, 100);
    }
    
    return HAL_ERROR;
}

/**
 * @brief UART接收完成中断回调（需在stm32f1xx_it.c中调用）
 * @param huart UART句柄指针
 * @note 该函数会在HAL_UART_RxCpltCallback中被调用
 */
void UART_Protocol_RxCallback(UART_HandleTypeDef *huart)
{
    if (huart != g_pUartHandle) {
        // 不是我们管理的UART，重启接收后返回
        HAL_UART_Receive_IT(huart, &g_RxLineBuffer.rxByte, 1);
        return;
    }
    
    uint8_t receivedByte = g_RxLineBuffer.rxByte;
    
    // 如果上一帧还未处理，丢弃当前字节
    if (g_RxLineBuffer.state == UART_RX_COMPLETE) {
        HAL_UART_Receive_IT(huart, &g_RxLineBuffer.rxByte, 1);
        return;
    }
    
    // 处理接收的字节
    if (receivedByte == '\n') {
        // 检测到换行符，标记帧接收完成
        g_RxLineBuffer.buffer[g_RxLineBuffer.index] = '\0';  // 字符串结束符
        g_RxLineBuffer.state = UART_RX_COMPLETE;
    }
    else if (receivedByte == '\r') {
        // 忽略回车符（不存入缓冲区）
    }
    else {
        // 正常字符，存入缓冲区
        if (g_RxLineBuffer.index < UART_BUFFER_SIZE - 1) {
            g_RxLineBuffer.buffer[g_RxLineBuffer.index++] = receivedByte;
            g_RxLineBuffer.state = UART_RX_RECEIVING;
        }
        else {
            // 缓冲区溢出，重置
            g_RxLineBuffer.index = 0;
            g_RxLineBuffer.state = UART_RX_IDLE;
        }
    }
    
    // 重启下一次接收
    HAL_UART_Receive_IT(huart, &g_RxLineBuffer.rxByte, 1);
}
