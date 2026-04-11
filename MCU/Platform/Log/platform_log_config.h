/**
 ******************************************************************************
 * @file    platform_log_config.h
 * @brief   平台日志输出配置（选择 USART1 / USART2 / USB CDC，及 USART 是否用 DMA）
 * @note    输出后端、DMA 等在此配置；总开关默认跟随 Platform/Config/platform_config.h 的 PLATFORM_DEBUG_LOG_ENABLE。
 ******************************************************************************
 */

#ifndef __PLATFORM_LOG_CONFIG_H__
#define __PLATFORM_LOG_CONFIG_H__

#include "../Config/platform_config.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================= 总开关 ========================= */
/**
 * 默认跟随 platform_config.h 中的 PLATFORM_DEBUG_LOG_ENABLE。
 * 若需单独覆盖，可在包含本头文件之前 #define PLATFORM_LOG_ENABLE。
 */
#ifndef PLATFORM_LOG_ENABLE
#define PLATFORM_LOG_ENABLE        PLATFORM_DEBUG_LOG_ENABLE
#endif

/** 1: 提供强符号 __io_putchar，将 printf/puts 等导向 PlatformLog */
#ifndef PLATFORM_LOG_HOOK_STDIO
#define PLATFORM_LOG_HOOK_STDIO    1
#endif

/**
 * 1: 启用 LOG_TEST / PLATFORM_LOG_TEST（传感器台架、原始数据等 printf 调试输出）
 * 0: 编译期剔除
 * 默认跟随 PLATFORM_DEBUG_LOG_ENABLE；可在包含本头文件之前单独覆盖。
 */
#ifndef PLATFORM_LOG_TEST_ENABLE
#define PLATFORM_LOG_TEST_ENABLE   PLATFORM_DEBUG_LOG_ENABLE
#endif

/* ========================= 输出后端 ========================= */
#define PLATFORM_LOG_BACKEND_USART1    0
#define PLATFORM_LOG_BACKEND_USART2    1
#define PLATFORM_LOG_BACKEND_CDC       2

#ifndef PLATFORM_LOG_BACKEND
#define PLATFORM_LOG_BACKEND       PLATFORM_LOG_BACKEND_USART1
#endif

/**
 * USART 发送方式（仅 PLATFORM_LOG_BACKEND 为 USART1/2 时有效；CDC 忽略此项）
 * 本工程 Cube 配置：USART2 已链接 TX DMA；USART1 无 TX DMA。
 * USART1 下若置 1 将在编译期报错，请改为 0 或为 USART1 在 Cube 中增加 TX DMA。
 */
#ifndef PLATFORM_LOG_USE_DMA
#define PLATFORM_LOG_USE_DMA       0
#endif

#ifndef PLATFORM_LOG_UART_TIMEOUT_MS
#define PLATFORM_LOG_UART_TIMEOUT_MS  100U
#endif

/** DMA 单次发送最大长度（静态缓冲）；更长则分片 */
#ifndef PLATFORM_LOG_DMA_MAX_CHUNK
#define PLATFORM_LOG_DMA_MAX_CHUNK    256U
#endif

/* ========================= 日志正文前缀（每条 PlatformLog_Write 先发送） ========================= */
/** 1: 在每次 PlatformLog_Write 的数据前附加 PLATFORM_LOG_PREFIX_STRING（printf/__io_putchar 不加此前缀） */
#ifndef PLATFORM_LOG_PREFIX_ENABLE
#define PLATFORM_LOG_PREFIX_ENABLE    0
#endif

/** 前缀字符串，须为字符串字面量（不要写成字符数组初始化形式） */
#ifndef PLATFORM_LOG_PREFIX_STRING
#define PLATFORM_LOG_PREFIX_STRING    "[PLOG] "
#endif

/* ========================= 配置校验 ========================= */
#if PLATFORM_LOG_ENABLE && PLATFORM_LOG_USE_DMA && (PLATFORM_LOG_BACKEND == PLATFORM_LOG_BACKEND_USART1)
#error "PLATFORM_LOG_USE_DMA on USART1: 当前工程未为 USART1 配置 TX DMA，请设 PLATFORM_LOG_USE_DMA 为 0 或改用 USART2。"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_LOG_CONFIG_H__ */
