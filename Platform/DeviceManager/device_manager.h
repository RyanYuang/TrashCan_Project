/**
 ******************************************************************************
 * @file    device_manager.h
 * @brief   设备管理器 - 统一管理所有硬件设备
 * @note    提供设备注册、初始化、数据读取的统一接口
 ******************************************************************************
 */

#ifndef __DEVICE_MANAGER_H__
#define __DEVICE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "platform_config.h"

/* ========================= 设备类型定义 ========================= */
typedef enum {
    DEVICE_TYPE_SENSOR = 0,         // 传感器类设备
    DEVICE_TYPE_ACTUATOR,           // 执行器类设备
    DEVICE_TYPE_DISPLAY,            // 显示类设备
    DEVICE_TYPE_COMMUNICATION,      // 通信类设备
    DEVICE_TYPE_MAX
} DeviceType_t;

/* ========================= 设备ID定义 ========================= */
typedef enum {
    DEVICE_ID_OLED = 0,             // OLED显示屏
    DEVICE_ID_AHT30,                // 温湿度传感器
    DEVICE_ID_GY302,                // 光照传感器
    DEVICE_ID_MQ2,                  // 气体传感器
    DEVICE_ID_ULTRASONIC,           // 超声波传感器
    DEVICE_ID_TB6612,               // 电机驱动
    DEVICE_ID_RGB,                  // RGB LED
    DEVICE_ID_BEEP,                 // 蜂鸣器
    DEVICE_ID_LORA,                 // LoRa模块
    DEVICE_ID_MAX
} DeviceID_t;

/* ========================= 设备状态定义 ========================= */
typedef enum {
    DEVICE_STATUS_UNINIT = 0,       // 未初始化
    DEVICE_STATUS_READY,            // 就绪
    DEVICE_STATUS_BUSY,             // 忙碌
    DEVICE_STATUS_ERROR,            // 错误
    DEVICE_STATUS_DISABLED          // 禁用
} DeviceStatus_t;

/* ========================= 设备结构定义 ========================= */
/**
 * @brief 设备信息结构体
 */
typedef struct {
    DeviceID_t id;                  // 设备ID
    const char* name;               // 设备名称
    DeviceType_t type;              // 设备类型
    DeviceStatus_t status;          // 设备状态
    void* handle;                   // 设备句柄（指向具体设备驱动的句柄）
    void* config;                   // 设备配置（指向具体设备的配置结构）
} DeviceInfo_t;

/**
 * @brief 设备操作接口
 */
typedef struct {
    // 设备初始化函数
    bool (*init)(void* handle, void* config);
    
    // 设备反初始化函数
    bool (*deinit)(void* handle);
    
    // 设备读取函数（用于传感器等输入设备）
    bool (*read)(void* handle, void* data);
    
    // 设备写入函数（用于执行器等输出设备）
    bool (*write)(void* handle, void* data);
    
    // 设备控制函数（用于特殊控制命令）
    bool (*control)(void* handle, uint32_t cmd, void* param);
} DeviceOps_t;

/**
 * @brief 完整的设备描述符
 */
typedef struct {
    DeviceInfo_t info;              // 设备信息
    DeviceOps_t ops;                // 设备操作接口
} DeviceDescriptor_t;

/* ========================= 设备管理器配置 ========================= */
#define DEVICE_MANAGER_MAX_DEVICES  16      // 最大设备数量

/* ========================= 设备管理器函数 ========================= */

/**
 * @brief 初始化设备管理器
 * @return true: 成功, false: 失败
 */
bool DeviceManager_Init(void);

/**
 * @brief 注册设备到设备管理器
 * @param descriptor 设备描述符指针
 * @return true: 成功, false: 失败
 */
bool DeviceManager_Register(DeviceDescriptor_t* descriptor);

/**
 * @brief 注销设备
 * @param id 设备ID
 * @return true: 成功, false: 失败
 */
bool DeviceManager_Unregister(DeviceID_t id);

/**
 * @brief 初始化指定设备
 * @param id 设备ID
 * @return true: 成功, false: 失败
 */
bool DeviceManager_InitDevice(DeviceID_t id);

/**
 * @brief 获取设备信息
 * @param id 设备ID
 * @return 设备信息指针，NULL表示设备不存在
 */
DeviceInfo_t* DeviceManager_GetDeviceInfo(DeviceID_t id);

/**
 * @brief 获取设备状态
 * @param id 设备ID
 * @return 设备状态
 */
DeviceStatus_t DeviceManager_GetDeviceStatus(DeviceID_t id);

/**
 * @brief 设置设备状态
 * @param id 设备ID
 * @param status 新状态
 * @return true: 成功, false: 失败
 */
bool DeviceManager_SetDeviceStatus(DeviceID_t id, DeviceStatus_t status);

/**
 * @brief 读取设备数据
 * @param id 设备ID
 * @param data 数据缓冲区
 * @return true: 成功, false: 失败
 */
bool DeviceManager_ReadDevice(DeviceID_t id, void* data);

/**
 * @brief 写入设备数据
 * @param id 设备ID
 * @param data 数据指针
 * @return true: 成功, false: 失败
 */
bool DeviceManager_WriteDevice(DeviceID_t id, void* data);

/**
 * @brief 控制设备
 * @param id 设备ID
 * @param cmd 控制命令
 * @param param 命令参数
 * @return true: 成功, false: 失败
 */
bool DeviceManager_ControlDevice(DeviceID_t id, uint32_t cmd, void* param);

/**
 * @brief 获取所有设备数量
 * @return 已注册的设备数量
 */
uint8_t DeviceManager_GetDeviceCount(void);

/**
 * @brief 列出所有设备
 * @param devices 设备信息数组
 * @param max_count 数组最大容量
 * @return 实际返回的设备数量
 */
uint8_t DeviceManager_ListDevices(DeviceInfo_t* devices, uint8_t max_count);

#ifdef __cplusplus
}
#endif

#endif /* __DEVICE_MANAGER_H__ */
