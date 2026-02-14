/**
  ******************************************************************************
  * @file           : threshold_config.h
  * @brief          : 传感器阈值配置模块头文件
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  * @description
  * 该模块用于管理温度、湿度、CO浓度、光照等传感器的阈值配置
  * 支持默认值初始化、配置更新和告警级别判断
  ******************************************************************************
  */

#ifndef __THRESHOLD_CONFIG_H
#define __THRESHOLD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief 告警级别枚举
 */
typedef enum {
    ALERT_NONE = 0,       // 正常
    ALERT_WARNING,        // 警告（CO达到warning阈值，或其他参数超出范围）
    ALERT_DANGER          // 危险（CO达到danger阈值）
} AlertLevel_t;

/**
 * @brief 阈值配置结构体
 * 存储所有传感器的上下限阈值
 */
typedef struct {
    float temp_low;       // 温度下限 (°C)
    float temp_high;      // 温度上限 (°C)
    float hum_low;        // 湿度下限 (%)
    float hum_high;       // 湿度上限 (%)
    float co_warning;     // CO警告阈值 (ppm)
    float co_danger;      // CO危险阈值 (ppm)
    float light_low;      // 光照下限 (lux)
    float light_high;     // 光照上限 (lux)
} ThresholdConfig_t;

/**
 * @brief 传感器数据结构体
 */
typedef struct {
    float temperature;    // 温度 (°C)
    float humidity;       // 湿度 (%)
    float co_concentration; // CO浓度 (ppm)
    float light_intensity;  // 光照强度 (lux)
} SensorData_t;

/* Exported constants --------------------------------------------------------*/

// 默认阈值定义
#define DEFAULT_TEMP_LOW        -10.0f
#define DEFAULT_TEMP_HIGH       45.0f
#define DEFAULT_HUM_LOW         20.0f
#define DEFAULT_HUM_HIGH        90.0f
#define DEFAULT_CO_WARNING      30.0f
#define DEFAULT_CO_DANGER       50.0f
#define DEFAULT_LIGHT_LOW       0.0f
#define DEFAULT_LIGHT_HIGH      10000.0f

/* Exported macro ------------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief 初始化阈值配置（使用默认值）
 * @param config 指向阈值配置结构体的指针
 */
void ThresholdConfig_Init(ThresholdConfig_t *config);

/**
 * @brief 应用新的阈值配置
 * @param config 新的阈值配置
 */
void ThresholdConfig_Apply(const ThresholdConfig_t *config);

/**
 * @brief 获取当前阈值配置
 * @return 当前阈值配置的指针（只读）
 */
const ThresholdConfig_t* ThresholdConfig_Get(void);

/**
 * @brief 检查传感器数据是否超出阈值
 * @param data 传感器数据
 * @return 告警级别
 */
AlertLevel_t ThresholdConfig_CheckAlert(const SensorData_t *data);

/**
 * @brief 获取告警级别的字符串描述
 * @param level 告警级别
 * @return 告警级别字符串
 */
const char* ThresholdConfig_GetAlertString(AlertLevel_t level);

#ifdef __cplusplus
}
#endif

#endif /* __THRESHOLD_CONFIG_H */
