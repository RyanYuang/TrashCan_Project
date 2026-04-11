#ifndef __JC278A_H__
#define __JC278A_H__

#include <stdint.h>
#include "main.h"  // 包含 HAL 库，定义 UART_HandleTypeDef
#include "Drivers.h"
#include "usart.h"

// ********** Marco ********** //
#define JC278A_MAX_NUM 5                 // JC278A设备最大数量
#define JC278A_DEFAULT_TIMEOUT 1000       // 默认超时时间(ms)
#define JC278A_RX_BUFFER_SIZE 256        // 接收缓冲区大小

// ********** Enum Definitions ********** //
typedef enum {
    JC278A_Status_OK = 0,                // JC278A工作正常 
    JC278A_Status_Error = 1,             // JC278A工作异常
    JC278A_Status_Timeout = 2,           // JC278A操作超时
    JC278A_Status_Busy = 3,              // JC278A忙
    JC278A_Status_NoResponse = 4,        // JC278A无响应
    JC278A_Status_RESERVED
} JC278A_Status_Enum;

// ********** Handle Definition ********** //
typedef struct {
    uint16_t ID;
} JC278A_Handle;

// ********** External API ********** //

// Device Driver
/* @brief JC278A设备检测
 * @param handler JC278A设备句柄
 * @retval JC278A_Status_Enum 发现设备返回JC278A_Status_OK，未发现设备返回JC278A_Status_Error
 */
JC278A_Status_Enum JC278A_Device_Detection(JC278A_Handle* handler);

/* @brief 创建JC278A设备
 * @param handler JC278A设备句柄指针
 * @param uart_handle UART句柄指针（如&huart1, &huart2等）
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Device_Create(JC278A_Handle* handler, UART_HandleTypeDef* uart_handle);

/* @brief 初始化JC278A设备
 * @param handler JC278A设备句柄指针
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Init(JC278A_Handle* handler);

/* @brief 发送数据到JC278A
 * @param handler JC278A设备句柄指针
 * @param data 要发送的数据指针
 * @param length 数据长度
 * @param timeout 超时时间(ms)，0表示使用默认超时时间
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Send_Data(JC278A_Handle* handler, const uint8_t* data, uint16_t length, uint32_t timeout);

/* @brief 从JC278A接收数据
 * @param handler JC278A设备句柄指针
 * @param data 接收数据缓冲区指针
 * @param length 期望接收的数据长度
 * @param timeout 超时时间(ms)，0表示使用默认超时时间
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Receive_Data(JC278A_Handle* handler, uint8_t* data, uint16_t* length, uint32_t timeout);

/* @brief 发送命令并接收响应（阻塞模式）
 * @param handler JC278A设备句柄指针
 * @param cmd 命令数据指针
 * @param cmd_len 命令长度
 * @param response 响应缓冲区指针
 * @param response_len 响应缓冲区大小（输入），实际接收长度（输出）
 * @param timeout 超时时间(ms)，0表示使用默认超时时间
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Send_Command(JC278A_Handle* handler, const uint8_t* cmd, uint16_t cmd_len, 
                                       uint8_t* response, uint16_t* response_len, uint32_t timeout);

/* @brief 复位JC278A设备
 * @param handler JC278A设备句柄指针
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Reset(JC278A_Handle* handler);

/* @brief 设置JC278A工作模式
 * @param handler JC278A设备句柄指针
 * @param mode 工作模式（根据JC278A协议定义）
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Set_Mode(JC278A_Handle* handler, uint8_t mode);

/* @brief 获取JC278A设备信息
 * @param handler JC278A设备句柄指针
 * @param info 信息缓冲区指针
 * @param info_len 信息缓冲区大小（输入），实际接收长度（输出）
 * @retval JC278A_Status_Enum 操作状态
 */
JC278A_Status_Enum JC278A_Get_Info(JC278A_Handle* handler, uint8_t* info, uint16_t* info_len);

#endif /* __JC278A_H__ */

