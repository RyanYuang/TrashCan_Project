/**
  ******************************************************************************
  * @file           : threshold_config.c
  * @brief          : 传感器阈值配置模块实现
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "threshold_config.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 全局阈值配置（当前生效的配置）
static ThresholdConfig_t g_CurrentConfig;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化阈值配置（使用默认值）
 * @param config 指向阈值配置结构体的指针
 */
void ThresholdConfig_Init(ThresholdConfig_t *config)
{
    if (config == NULL) {
        config = &g_CurrentConfig;
    }
    
    config->temp_low = DEFAULT_TEMP_LOW;
    config->temp_high = DEFAULT_TEMP_HIGH;
    config->hum_low = DEFAULT_HUM_LOW;
    config->hum_high = DEFAULT_HUM_HIGH;
    config->co_warning = DEFAULT_CO_WARNING;
    config->co_danger = DEFAULT_CO_DANGER;
    config->light_low = DEFAULT_LIGHT_LOW;
    config->light_high = DEFAULT_LIGHT_HIGH;
    
    // 如果传入的是外部config，也更新全局配置
    if (config != &g_CurrentConfig) {
        memcpy(&g_CurrentConfig, config, sizeof(ThresholdConfig_t));
    }
}

/**
 * @brief 应用新的阈值配置
 * @param config 新的阈值配置
 */
void ThresholdConfig_Apply(const ThresholdConfig_t *config)
{
    if (config == NULL) {
        return;
    }
    
    // 复制到全局配置
    memcpy(&g_CurrentConfig, config, sizeof(ThresholdConfig_t));
    
    // TODO: 如果需要持久化存储，在此处写入Flash/EEPROM
    // Flash_Write(THRESHOLD_CONFIG_ADDR, &g_CurrentConfig, sizeof(ThresholdConfig_t));
}

/**
 * @brief 获取当前阈值配置
 * @return 当前阈值配置的指针（只读）
 */
const ThresholdConfig_t* ThresholdConfig_Get(void)
{
    return &g_CurrentConfig;
}

/**
 * @brief 检查传感器数据是否超出阈值
 * @param data 传感器数据
 * @return 告警级别
 */
AlertLevel_t ThresholdConfig_CheckAlert(const SensorData_t *data)
{
    if (data == NULL) {
        return ALERT_NONE;
    }
    
    // 优先级1: 检查CO危险级别
    if (data->co_concentration >= g_CurrentConfig.co_danger) {
        return ALERT_DANGER;
    }
    
    // 优先级2: 检查CO警告级别
    if (data->co_concentration >= g_CurrentConfig.co_warning) {
        return ALERT_WARNING;
    }
    
    // 优先级3: 检查其他参数是否超出正常范围
    if (data->temperature < g_CurrentConfig.temp_low || 
        data->temperature > g_CurrentConfig.temp_high) {
        return ALERT_WARNING;
    }
    
    if (data->humidity < g_CurrentConfig.hum_low || 
        data->humidity > g_CurrentConfig.hum_high) {
        return ALERT_WARNING;
    }
    
    if (data->light_intensity < g_CurrentConfig.light_low || 
        data->light_intensity > g_CurrentConfig.light_high) {
        return ALERT_WARNING;
    }
    
    return ALERT_NONE;
}

/**
 * @brief 获取告警级别的字符串描述
 * @param level 告警级别
 * @return 告警级别字符串
 */
const char* ThresholdConfig_GetAlertString(AlertLevel_t level)
{
    switch (level) {
        case ALERT_NONE:
            return "Normal";
        case ALERT_WARNING:
            return "Warning";
        case ALERT_DANGER:
            return "Danger";
        default:
            return "Unknown";
    }
}
