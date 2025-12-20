#ifndef __GY302_H__
#define __GY302_H__

#include <stdint.h>
#include "Drivers.h"

// ********** Marco ********** //
#define GY302_MAX_NUM 5                 // GY302设备最大数量

// ********** Enum Definitions ********** //
typedef enum {
    GY302_Status_OK = 0,                // GY302工作正常 
    GY302_Status_Error = 1,             // GY302工作异常
    GY302_Status_Timeout = 2,           // GY302操作超时
    GY302_Status_Busy = 3,              // GY302忙
    GY302_Status_RESERVED
} GY302_Status_Enum;

// ********** Handle Definition ********** //
typedef struct {
    uint16_t ID;
} GY302_Handle;

// ********** External API ********** //
/* @brief 创建GY302设备
 * @param handler GY302设备句柄指针
 * @param SDA SDA引脚结构体指针
 * @param SCL SCL引存结构体指针
 * @param address GY302设备I2C地址
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Device_Create(GY302_Handle* handler, Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address);

/* @brief 初始化GY302设备
 * @param handler GY302设备句柄指针
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Init(GY302_Handle* handler);

/* @brief 从GY302读取光照强度值
 * @param handler GY302设备句柄指针
 * @param lux 指向存储光照强度值的指针
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Read_Lux(GY302_Handle* handler, float* lux);

/* @brief 设置GY302测量模式
 * @param handler GY302设备句柄指针
 * @param mode 测量模式 (0:断电, 1:持续H分辨率, 2:持续H分辨率2, 3:持续L分辨率, 
 *              4:单次H分辨率, 5:单次H分辨率2, 6:单次L分辨率)
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Set_Mode(GY302_Handle* handler, uint8_t mode);

/* @brief 设置GY302测量时间
 * @param handler GY302设备句柄指针
 * @param time 测量时间 (0-254)
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Set_Measurement_Time(GY302_Handle* handler, uint8_t time);

/* @brief 复位GY302设备
 * @param handler GY302设备句柄指针
 * @retval GY302_Status_Enum 操作状态
 */
GY302_Status_Enum GY302_Reset(GY302_Handle* handler);

#endif /* __GY302_H__ */