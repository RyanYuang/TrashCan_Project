/*
 * aht30.h
 *
 *  Created on: Nov 28, 2025
 *      Author: zeng
 */

#ifndef INC_AHT30_H_
#define INC_AHT30_H_

#include <stdint.h>
#include "Drivers.h"

// ********** Marco ********** //
#define AHT30_MAX_NUM 5                 // AHT30设备最大数量
#define AHT30_ADDRESS 0x38              // AHT30设备I2C地址

// ********** Enum Definitions ********** //
typedef enum {
    AHT30_Status_OK = 0,                // AHT30工作正常 
    AHT30_Status_Error = 1,             // AHT30工作异常
    AHT30_Status_Timeout = 2,           // AHT30操作超时
    AHT30_Status_Busy = 3,              // AHT30忙
    AHT30_Status_Uninitialized = 4,     // AHT30未初始化
    AHT30_Status_RESERVED
} AHT30_Status_Enum;

// ********** Handle Definition ********** //
typedef struct {
    uint16_t ID;
} AHT30_Handle;

// ********** External API ********** //
/* @brief 创建AHT30设备
 * @param handler AHT30设备句柄指针
 * @param SDA SDA引脚结构体指针
 * @param SCL SCL引脚结构体指针
 * @param address AHT30设备I2C地址
 * @retval AHT30_Status_Enum 操作状态
 */
AHT30_Status_Enum AHT30_Device_Create(AHT30_Handle* handler, Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address);

/* @brief 初始化AHT30设备
 * @param handler AHT30设备句柄指针
 * @retval AHT30_Status_Enum 操作状态
 */
AHT30_Status_Enum AHT30_Init(AHT30_Handle* handler);

/* @brief 从AHT30读取温湿度数据
 * @param handler AHT30设备句柄指针
 * @param temperature 指向存储温度值的指针
 * @param humidity 指向存储湿度值的指针
 * @retval AHT30_Status_Enum 操作状态
 */
AHT30_Status_Enum AHT30_Read_Data(AHT30_Handle* handler, float* temperature, float* humidity);

/* @brief 复位AHT30设备
 * @param handler AHT30设备句柄指针
 * @retval AHT30_Status_Enum 操作状态
 */
AHT30_Status_Enum AHT30_Reset(AHT30_Handle* handler);

/* @brief 获取AHT30设备状态
 * @param handler AHT30设备句柄指针
 * @retval AHT30_Status_Enum 操作状态
 */
AHT30_Status_Enum AHT30_Get_Status(AHT30_Handle* handler);

#endif /* INC_AHT30_H_ */