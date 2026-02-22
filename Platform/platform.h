/**
 ******************************************************************************
 * @file    platform.h
 * @brief   平台总头文件 - 统一包含所有平台组件
 * @note    项目只需要包含这一个头文件即可使用整个平台
 ******************************************************************************
 */

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= 平台配置 ========================= */
#include "platform_config.h"
#include "project_config.h"

/* ========================= 设备管理 ========================= */
#include "device_manager.h"
#include "device_config.h"

/* ========================= 应用框架 ========================= */
#include "app_framework.h"

/* ========================= 平台信息 ========================= */
#define PLATFORM_NAME       "STM32 IoT Platform"
#define PLATFORM_AUTHOR     "Your Name"
#define PLATFORM_DATE       "2026-02-18"

/**
 * @brief 平台初始化
 * @note 初始化设备管理器和应用框架
 * @return true: 成功, false: 失败
 */
static inline bool Platform_Init(void)
{
    // 初始化设备管理器
    if(!DeviceManager_Init()) {
        return false;
    }
    
    return true;
}

/**
 * @brief 打印平台信息
 */
static inline void Platform_PrintInfo(void)
{
#if PLATFORM_DEBUG_ENABLE
    printf("\r\n");
    printf("========================================\r\n");
    printf("  %s\r\n", PLATFORM_NAME);
    printf("  Version: %s\r\n", PLATFORM_VERSION_STRING);
    printf("  Date: %s\r\n", PLATFORM_DATE);
    printf("  MCU: STM32F103RCT6\r\n");
    printf("  Clock: %ld Hz\r\n", PLATFORM_SYSTEM_CLOCK_HZ);
    printf("========================================\r\n");
    printf("\r\n");
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_H__ */
