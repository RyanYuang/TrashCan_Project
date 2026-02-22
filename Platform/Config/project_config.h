/**
 ******************************************************************************
 * @file    project_config.h
 * @brief   项目配置头文件 - 每个具体项目的配置
 * @note    这个文件由具体项目定义，覆盖平台默认配置
 ******************************************************************************
 */

#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "platform_config.h"

/* ========================= 项目基本信息 ========================= */
// 项目名称
#define PROJECT_NAME                "TrashCan_Project"

// 项目版本
#define PROJECT_VERSION             "1.0.0"

// 项目类型
#define PROJECT_TYPE                PROJECT_TYPE_TRASH_CAN

// 项目描述
#define PROJECT_DESCRIPTION         "Smart Trash Can with Environmental Monitoring"

/* ========================= 项目特定配置 ========================= */
// 项目启用的设备（可以覆盖平台配置）
#define PROJECT_USE_OLED            PLATFORM_DEVICE_OLED
#define PROJECT_USE_AHT30           PLATFORM_DEVICE_AHT30
#define PROJECT_USE_GY302           PLATFORM_DEVICE_GY302
#define PROJECT_USE_MQ2             PLATFORM_DEVICE_MQ2
#define PROJECT_USE_TB6612          PLATFORM_DEVICE_TB6612
#define PROJECT_USE_RGB             PLATFORM_DEVICE_RGB
#define PROJECT_USE_BEEP            PLATFORM_DEVICE_BEEP
#define PROJECT_USE_ULTRASONIC      PLATFORM_DEVICE_ULTRASONIC
#define PROJECT_USE_LORA            PLATFORM_DEVICE_LORA

/* ========================= 项目功能配置 ========================= */
// 阈值报警功能
#define PROJECT_FEATURE_THRESHOLD_ALARM     1

// 串口协议通信
#define PROJECT_FEATURE_UART_PROTOCOL       1

// 数据上报功能
#define PROJECT_FEATURE_DATA_REPORT         1

// 远程控制功能
#define PROJECT_FEATURE_REMOTE_CONTROL      1

/* ========================= 项目引脚配置 ========================= */
// 说明：实际的引脚配置应该在设备初始化配置文件中定义
// 这里只做标识性说明，具体引脚由设备配置决定

/* ========================= 项目参数配置 ========================= */
// 传感器采样间隔
#define PROJECT_SENSOR_SAMPLE_INTERVAL_MS   10

// 数据上报周期（覆盖平台配置）
#ifndef PROJECT_DATA_REPORT_PERIOD_MS
#define PROJECT_DATA_REPORT_PERIOD_MS       PLATFORM_DATA_REPORT_PERIOD_MS
#endif

// 主循环延时
#ifndef PROJECT_MAIN_LOOP_DELAY_MS
#define PROJECT_MAIN_LOOP_DELAY_MS          PLATFORM_MAIN_LOOP_DELAY_MS
#endif

/* ========================= 项目调试配置 ========================= */
// 调试打印（继承平台配置）
#define PROJECT_DEBUG_ENABLE        PLATFORM_DEBUG_ENABLE

// 传感器数据调试打印
#define PROJECT_DEBUG_SENSOR_DATA   1

// 电机控制调试打印
#define PROJECT_DEBUG_MOTOR_CTRL    0

#ifdef __cplusplus
}
#endif

#endif /* __PROJECT_CONFIG_H__ */
