/**
 ******************************************************************************
 * @file    platform_log.h
 * @brief   平台统一日志输出 API
 * @note    配置见 platform_log_config.h。初始化外设后调用 PlatformLog_Init()（CDC 建议在 USB 初始化之后）。
 *          宏均以 PLATFORM_LOG_ 为前缀；LOG_WRITE / LOG_* 为兼容别名。
 *          传感器测试流水由 LOG_TEST 管理（见 PLATFORM_LOG_TEST_ENABLE）。
 ******************************************************************************
 */

#ifndef __PLATFORM_LOG_H__
#define __PLATFORM_LOG_H__

#include "platform_log_config.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  初始化平台日志模块（外设须已由 CubeMX 初始化）
 * @retval 无
 */
void PlatformLog_Init(void);

/**
 * @brief  输出一段原始日志数据，路由由 platform_log_config.h 决定
 * @param  data 数据缓冲区首地址
 * @param  len  字节数
 * @retval 无
 */
void PlatformLog_Write(const uint8_t *data, size_t len);

#if PLATFORM_LOG_ENABLE

/** 写出缓冲区（字节数由 len 指定） */
#define PLATFORM_LOG_WRITE(ptr, len)  PlatformLog_Write((const uint8_t *)(ptr), (size_t)(len))

/** 以空字符结尾的 C 字符串（内部使用 strlen 求长） */
#define PLATFORM_LOG_STR(s)  PLATFORM_LOG_WRITE((s), strlen(s))

/** 级别标签 + 字符串字面量 msg（msg 须为字面量，便于编译期求长） */
#define PLATFORM_LOG_INFO_LITERAL(msg)   PLATFORM_LOG_WRITE("[INFO] " msg, sizeof("[INFO] " msg) - 1U)
#define PLATFORM_LOG_WARN_LITERAL(msg)   PLATFORM_LOG_WRITE("[WARN] " msg, sizeof("[WARN] " msg) - 1U)
#define PLATFORM_LOG_ERROR_LITERAL(msg)  PLATFORM_LOG_WRITE("[ERROR] " msg, sizeof("[ERROR] " msg) - 1U)
#define PLATFORM_LOG_DEBUG_LITERAL(msg)  PLATFORM_LOG_WRITE("[DEBUG] " msg, sizeof("[DEBUG] " msg) - 1U)

/** 与 PLATFORM_LOG_*_LITERAL 等价，短名 */
#define LOG_INFO(msg)   PLATFORM_LOG_INFO_LITERAL(msg)
#define LOG_WARN(msg)   PLATFORM_LOG_WARN_LITERAL(msg)
#define LOG_ERROR(msg)  PLATFORM_LOG_ERROR_LITERAL(msg)
#define LOG_DEBUG(msg)  PLATFORM_LOG_DEBUG_LITERAL(msg)

#else

#define PLATFORM_LOG_WRITE(ptr, len)        ((void)0)
#define PLATFORM_LOG_STR(s)                 ((void)0)
#define PLATFORM_LOG_INFO_LITERAL(msg)      ((void)0)
#define PLATFORM_LOG_WARN_LITERAL(msg)      ((void)0)
#define PLATFORM_LOG_ERROR_LITERAL(msg)     ((void)0)
#define PLATFORM_LOG_DEBUG_LITERAL(msg)     ((void)0)
#define LOG_INFO(msg)                       ((void)0)
#define LOG_WARN(msg)                       ((void)0)
#define LOG_ERROR(msg)                      ((void)0)
#define LOG_DEBUG(msg)                      ((void)0)
#define LOG_WRITE(ptr, len)                 ((void)0)

#endif /* PLATFORM_LOG_ENABLE */

/**
 * 传感器 / 台架测试日志（可变参数，内部走 printf，若开启 PLATFORM_LOG_HOOK_STDIO 则经 PlatformLog 输出）
 * 由 platform_log_config.h 中 PLATFORM_LOG_TEST_ENABLE 总控；与 PLATFORM_LOG_ENABLE 同时为 1 才生效。
 */
#if PLATFORM_LOG_ENABLE && PLATFORM_LOG_TEST_ENABLE
#include <stdio.h>
#define LOG_TEST(fmt, ...)           printf("[LOG_TEST] " fmt, ##__VA_ARGS__)
#define PLATFORM_LOG_TEST(fmt, ...)  LOG_TEST(fmt, ##__VA_ARGS__)
#else
#define LOG_TEST(fmt, ...)           ((void)0)
#define PLATFORM_LOG_TEST(fmt, ...)  ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __PLATFORM_LOG_H__ */
