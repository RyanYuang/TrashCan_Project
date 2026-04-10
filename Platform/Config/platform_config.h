/**
 ******************************************************************************
 * @file    platform_config.h
 * @brief   平台配置头文件 - 定义平台级别的配置项
 * @note    这个文件定义了平台的基础配置，是所有项目的基线配置
 ******************************************************************************
 */

#ifndef __PLATFORM_CONFIG_H__
#define __PLATFORM_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= 平台版本信息 ========================= */
#define PLATFORM_VERSION_MAJOR    1
#define PLATFORM_VERSION_MINOR    0
#define PLATFORM_VERSION_PATCH    0
#define PLATFORM_VERSION_STRING   "1.0.0"

/* ========================= 平台基础配置 ========================= */
// MCU 型号
#define PLATFORM_MCU_STM32F103RC

// 系统时钟频率（Hz）
#define PLATFORM_SYSTEM_CLOCK_HZ    72000000

// 串口波特率配置
#define PLATFORM_UART_BAUDRATE_DEFAULT  115200

/* ========================= 调试配置 ========================= */
// 调试打印使能
#define PLATFORM_DEBUG_ENABLE       1

// 用户打印使能
#define PLATFORM_USER_PRINT_ENABLE  1

// 调试串口选择（默认UART1，与协议等旧代码共用句柄；printf 重定向见 Platform/Log/platform_log_config.h）
#define PLATFORM_DEBUG_UART         huart1

/* ========================= 功能模块使能 ========================= */
// IIC 总线使能
#define PLATFORM_IIC_ENABLE         1

// UART 协议使能
#define PLATFORM_UART_PROTOCOL_ENABLE   1

// ADC 使能
#define PLATFORM_ADC_ENABLE         1

// DMA 使能
#define PLATFORM_DMA_ENABLE         1

// 定时器使能
#define PLATFORM_TIMER_ENABLE       1

/* ========================= 设备支持配置 ========================= */
// OLED 显示器支持
#define PLATFORM_DEVICE_OLED        1

// 温湿度传感器支持（AHT30）
#define PLATFORM_DEVICE_AHT30       1

// 光照传感器支持（GY302/BH1750）
#define PLATFORM_DEVICE_GY302       1

// 气体传感器支持（MQ-2）
#define PLATFORM_DEVICE_MQ2         1

// 电机驱动支持（TB6612）
#define PLATFORM_DEVICE_TB6612      1

// RGB LED 支持
#define PLATFORM_DEVICE_RGB         1

// 蜂鸣器支持
#define PLATFORM_DEVICE_BEEP        1

// 超声波传感器支持
#define PLATFORM_DEVICE_ULTRASONIC  1

// LoRa模块支持（JC278A）
#define PLATFORM_DEVICE_LORA        1

/* ========================= 应用框架配置 ========================= */
// 数据上报周期（ms）
#define PLATFORM_DATA_REPORT_PERIOD_MS  2000

// 主循环延时（ms）
#define PLATFORM_MAIN_LOOP_DELAY_MS     10

// 传感器缓冲区大小
#define PLATFORM_SENSOR_BUFFER_SIZE     32

/* ========================= 内存配置 ========================= */
// ADC 缓冲区大小
#define PLATFORM_ADC_BUFFER_SIZE    1

// OLED 显示缓冲区大小
#define PLATFORM_OLED_BUFFER_SIZE   32

// 协议接收缓冲区大小
#define PLATFORM_PROTOCOL_RX_BUFFER_SIZE    128

/* ========================= 项目类型定义 ========================= */
/**
 * @brief 项目类型枚举
 * @note 用于标识当前运行的项目类型
 */
typedef enum {
    PROJECT_TYPE_TRASH_CAN = 0,     // 垃圾桶项目
    PROJECT_TYPE_ENV_CAR,           // 环境检测车项目
    PROJECT_TYPE_SMART_HOME,        // 智能家居项目
    PROJECT_TYPE_CUSTOM,            // 自定义项目
    PROJECT_TYPE_MAX
} ProjectType_t;

/* ========================= 配置校验 ========================= */
#if PLATFORM_DEBUG_ENABLE && !PLATFORM_UART_PROTOCOL_ENABLE
#warning "Debug is enabled but UART protocol is disabled"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_CONFIG_H__ */
