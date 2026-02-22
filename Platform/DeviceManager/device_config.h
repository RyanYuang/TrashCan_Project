/**
 ******************************************************************************
 * @file    device_config.h
 * @brief   设备配置接口 - 为每个项目提供设备配置模板
 * @note    项目可以通过这个文件配置需要使用的设备及其参数
 ******************************************************************************
 */

#ifndef __DEVICE_CONFIG_H__
#define __DEVICE_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "device_manager.h"
#include "gpio.h"

/* ========================= 设备配置结构体 ========================= */

/**
 * @brief IIC设备引脚配置
 */
typedef struct {
    GPIO_TypeDef* sda_port;
    uint16_t sda_pin;
    GPIO_TypeDef* scl_port;
    uint16_t scl_pin;
    uint8_t device_addr;
} IICDeviceConfig_t;

/**
 * @brief UART设备配置
 */
typedef struct {
    void* uart_handle;      // UART句柄指针
    uint32_t baudrate;
} UARTDeviceConfig_t;

/**
 * @brief PWM设备配置
 */
typedef struct {
    void* timer_handle;     // 定时器句柄指针
    uint32_t channel;
} PWMDeviceConfig_t;

/**
 * @brief GPIO设备配置
 */
typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
} GPIODeviceConfig_t;

/* ========================= 设备配置接口 ========================= */

/**
 * @brief 初始化所有项目设备配置
 * @note 项目应该实现这个函数来注册所有需要的设备
 * @return true: 成功, false: 失败
 */
bool DeviceConfig_InitAll(void);

/**
 * @brief 获取项目设备列表
 * @note 返回当前项目使用的设备ID列表
 * @param device_ids 设备ID数组
 * @param max_count 数组最大容量
 * @return 实际返回的设备数量
 */
uint8_t DeviceConfig_GetProjectDevices(DeviceID_t* device_ids, uint8_t max_count);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_CONFIG_H__ */
