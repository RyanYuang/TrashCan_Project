#ifndef _OLED_H
#define _OLED_H
#include <stdint.h>
#include "Drivers.h"

// ********** Marco ********** //
#define OLED_MAX_NUM 5               // OLED设备最大数量

// ********** Internal API ********** //

// 声明设备句柄
typedef struct OLED_Handle OLED_Handle;
typedef struct OLED_Device_Driver OLED_Device_Driver;
// OLED 枚举
typedef enum {
    OLED_Status_OK = 0,             // OELD工作正常 
    OLED_Status_Error = 1,          // OELD工作异常
    OELD_Status_Timeout = 2,        // OELD操作超时
    OLED_Status_RESERVED
} OLED_Status_Enum;




// ********** External API ********** //
//OLED设备句柄
typedef struct OLED_Handle{
    uint16_t ID;
} OLED_Handle;


// Device Drver
/* @brief OLED设备扫描
 * @param handler OLED设备句柄
 * 发现设备返回OLED_Status_OK，未发现设备返回OLED_Status_Error
 */
OLED_Status_Enum OLED_Device_Detection(OLED_Handle* handler);

/* @brief OLED设备驱动句柄
 * @param handler OLED设备句柄
 * @param reg 寄存器地址
 * @param Data 要写入的数据
 * @details 包含SDA和SCL引脚以及IIC地址
 */
OLED_Status_Enum OLED_IIC_Write_Reg(OLED_Device_Driver* handler,uint8_t reg,uint8_t Data);
/* @brief 从OLED设备读取寄存器数据
 * @param handler OLED设备句柄
 * @param reg 寄存器地址
 * @param Data 指向存储读取数据的变量指针
 * @retval 操作状态
 */
OLED_Status_Enum OLED_IIC_Read_Reg(OLED_Device_Driver* handler,uint8_t reg,uint8_t* Data);




// ********** North API ********** //
/* @brief 创建OLED设备
 * @param SDA  SDA引脚结构体
 * @param SCL  SCL引脚结构体
 * @param address OLED地址
 */
OLED_Status_Enum OLED_Device_Create(OLED_Handle* handler,Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address);
/* @brief 初始化OLED
 * @param None
 * @retval None
 */
OLED_Status_Enum OLED_Init(OLED_Handle* handler);
/* @brief 清除OLED屏幕
 * @param None
 * @retval None
 */
OLED_Status_Enum OLED_Clear(OLED_Handle* handler);
/* @brief 显示字符
 * @param handler OLED设备句柄
 * @param x    X坐标
 * @param y    Y坐标
 * @param chr  字符
 * @retval None
 */
OLED_Status_Enum OLED_ShowChar(OLED_Handle* handler, uint8_t x, uint8_t y, char chr);
/* @brief 显示字符串
 * @param handler OLED设备句柄
 * @param x    X坐标
 * @param y    Y坐标
 * @param str  字符串指针
 * @param font_size 字体高度，目前仅支持16
 * @retval None
 */
OLED_Status_Enum OLED_ShowString(OLED_Handle* handler, uint8_t x, uint8_t y, const char *str, uint8_t font_size);
/* @brief 显示数字
 * @param handler OLED设备句柄
 * @param x    X坐标
 * @param y    Y坐标
 * @param num  数字
 * @param len  数字长度
 * @retval None
 */
OLED_Status_Enum OLED_ShowNum(OLED_Handle* handler, uint8_t x, uint8_t y, uint32_t num, uint8_t len);


#endif
