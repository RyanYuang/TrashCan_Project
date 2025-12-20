#ifndef __DRIVERS_H__
#define __DRIVERS_H__
#include "main.h"
// 通用引脚结构体 
typedef struct { GPIO_TypeDef* Port; uint16_t Pin; } Pin_Struct;

// 外设驱动
#include "Driver_IIC/IIC_Platform/CN/IIC_Platform_CN.h"



#endif
