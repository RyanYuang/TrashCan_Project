/**
 ******************************************************************************
 * @file    device_manager.c
 * @brief   设备管理器实现
 ******************************************************************************
 */

#include "device_manager.h"
#include <string.h>
#include <stdio.h>

/* ========================= 私有变量 ========================= */
static DeviceDescriptor_t* g_devices[DEVICE_MANAGER_MAX_DEVICES];
static uint8_t g_device_count = 0;
static bool g_manager_initialized = false;

/* ========================= 设备管理器实现 ========================= */

/**
 * @brief 初始化设备管理器
 */
bool DeviceManager_Init(void)
{
    if(g_manager_initialized) {
        return true;
    }
    
    // 清空设备列表
    memset(g_devices, 0, sizeof(g_devices));
    g_device_count = 0;
    g_manager_initialized = true;
    
#if PLATFORM_DEBUG_ENABLE
    printf("[DeviceManager] Device Manager Initialized\r\n");
#endif
    
    return true;
}

/**
 * @brief 注册设备到设备管理器
 */
bool DeviceManager_Register(DeviceDescriptor_t* descriptor)
{
    if(!g_manager_initialized) {
        DeviceManager_Init();
    }
    
    if(descriptor == NULL) {
        return false;
    }
    
    if(g_device_count >= DEVICE_MANAGER_MAX_DEVICES) {
#if PLATFORM_DEBUG_ENABLE
        printf("[DeviceManager] Error: Device list is full\r\n");
#endif
        return false;
    }
    
    // 检查设备ID是否已存在
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == descriptor->info.id) {
#if PLATFORM_DEBUG_ENABLE
            printf("[DeviceManager] Warning: Device ID %d already registered\r\n", descriptor->info.id);
#endif
            return false;
        }
    }
    
    // 添加设备到列表
    g_devices[g_device_count] = descriptor;
    g_device_count++;
    
#if PLATFORM_DEBUG_ENABLE
    printf("[DeviceManager] Registered device: %s (ID: %d)\r\n", 
           descriptor->info.name, descriptor->info.id);
#endif
    
    return true;
}

/**
 * @brief 注销设备
 */
bool DeviceManager_Unregister(DeviceID_t id)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            // 如果设备已初始化，先反初始化
            if(g_devices[i]->info.status != DEVICE_STATUS_UNINIT) {
                if(g_devices[i]->ops.deinit != NULL) {
                    g_devices[i]->ops.deinit(g_devices[i]->info.handle);
                }
            }
            
            // 移除设备（将后面的设备前移）
            for(uint8_t j = i; j < g_device_count - 1; j++) {
                g_devices[j] = g_devices[j + 1];
            }
            g_devices[g_device_count - 1] = NULL;
            g_device_count--;
            
#if PLATFORM_DEBUG_ENABLE
            printf("[DeviceManager] Unregistered device ID: %d\r\n", id);
#endif
            return true;
        }
    }
    
    return false;
}

/**
 * @brief 初始化指定设备
 */
bool DeviceManager_InitDevice(DeviceID_t id)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            DeviceDescriptor_t* dev = g_devices[i];
            
            if(dev->info.status != DEVICE_STATUS_UNINIT) {
#if PLATFORM_DEBUG_ENABLE
                printf("[DeviceManager] Device %s already initialized\r\n", dev->info.name);
#endif
                return true;
            }
            
            if(dev->ops.init != NULL) {
                bool result = dev->ops.init(dev->info.handle, dev->info.config);
                if(result) {
                    dev->info.status = DEVICE_STATUS_READY;
#if PLATFORM_DEBUG_ENABLE
                    printf("[DeviceManager] Device %s initialized successfully\r\n", dev->info.name);
#endif
                } else {
                    dev->info.status = DEVICE_STATUS_ERROR;
#if PLATFORM_DEBUG_ENABLE
                    printf("[DeviceManager] Device %s initialization failed\r\n", dev->info.name);
#endif
                }
                return result;
            }
            
            return false;
        }
    }
    
    return false;
}

/**
 * @brief 获取设备信息
 */
DeviceInfo_t* DeviceManager_GetDeviceInfo(DeviceID_t id)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            return &(g_devices[i]->info);
        }
    }
    return NULL;
}

/**
 * @brief 获取设备状态
 */
DeviceStatus_t DeviceManager_GetDeviceStatus(DeviceID_t id)
{
    DeviceInfo_t* info = DeviceManager_GetDeviceInfo(id);
    if(info != NULL) {
        return info->status;
    }
    return DEVICE_STATUS_ERROR;
}

/**
 * @brief 设置设备状态
 */
bool DeviceManager_SetDeviceStatus(DeviceID_t id, DeviceStatus_t status)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            g_devices[i]->info.status = status;
            return true;
        }
    }
    return false;
}

/**
 * @brief 读取设备数据
 */
bool DeviceManager_ReadDevice(DeviceID_t id, void* data)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            DeviceDescriptor_t* dev = g_devices[i];
            
            if(dev->info.status != DEVICE_STATUS_READY) {
                return false;
            }
            
            if(dev->ops.read != NULL) {
                return dev->ops.read(dev->info.handle, data);
            }
            
            return false;
        }
    }
    return false;
}

/**
 * @brief 写入设备数据
 */
bool DeviceManager_WriteDevice(DeviceID_t id, void* data)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            DeviceDescriptor_t* dev = g_devices[i];
            
            if(dev->info.status != DEVICE_STATUS_READY) {
                return false;
            }
            
            if(dev->ops.write != NULL) {
                return dev->ops.write(dev->info.handle, data);
            }
            
            return false;
        }
    }
    return false;
}

/**
 * @brief 控制设备
 */
bool DeviceManager_ControlDevice(DeviceID_t id, uint32_t cmd, void* param)
{
    for(uint8_t i = 0; i < g_device_count; i++) {
        if(g_devices[i] != NULL && g_devices[i]->info.id == id) {
            DeviceDescriptor_t* dev = g_devices[i];
            
            if(dev->ops.control != NULL) {
                return dev->ops.control(dev->info.handle, cmd, param);
            }
            
            return false;
        }
    }
    return false;
}

/**
 * @brief 获取所有设备数量
 */
uint8_t DeviceManager_GetDeviceCount(void)
{
    return g_device_count;
}

/**
 * @brief 列出所有设备
 */
uint8_t DeviceManager_ListDevices(DeviceInfo_t* devices, uint8_t max_count)
{
    uint8_t count = (g_device_count < max_count) ? g_device_count : max_count;
    
    for(uint8_t i = 0; i < count; i++) {
        if(g_devices[i] != NULL) {
            memcpy(&devices[i], &(g_devices[i]->info), sizeof(DeviceInfo_t));
        }
    }
    
    return count;
}
