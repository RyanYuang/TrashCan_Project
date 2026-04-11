/**
 ******************************************************************************
 * @file    app_framework.h
 * @brief   应用框架头文件 - 提供应用层开发框架
 * @note    为所有项目提供统一的应用开发接口和生命周期管理
 ******************************************************************************
 */

#ifndef __APP_FRAMEWORK_H__
#define __APP_FRAMEWORK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "platform_config.h"

/* ========================= 应用状态定义 ========================= */
typedef enum {
    APP_STATE_UNINIT = 0,           // 未初始化
    APP_STATE_INIT,                 // 初始化中
    APP_STATE_RUNNING,              // 运行中
    APP_STATE_PAUSED,               // 暂停
    APP_STATE_ERROR,                // 错误
    APP_STATE_SHUTDOWN              // 关闭
} AppState_t;

/* ========================= 应用生命周期回调 ========================= */
/**
 * @brief 应用生命周期回调函数结构体
 */
typedef struct {
    /**
     * @brief 应用初始化回调
     * @note 在应用启动时调用一次，用于初始化应用特定的资源
     * @return true: 成功, false: 失败
     */
    bool (*on_init)(void);
    
    /**
     * @brief 应用启动回调
     * @note 在应用初始化成功后调用
     */
    void (*on_start)(void);
    
    /**
     * @brief 应用循环回调
     * @note 在主循环中被周期性调用，用于执行应用的主要逻辑
     */
    void (*on_loop)(void);
    
    /**
     * @brief 应用暂停回调
     * @note 在应用暂停时调用
     */
    void (*on_pause)(void);
    
    /**
     * @brief 应用恢复回调
     * @note 在应用从暂停状态恢复时调用
     */
    void (*on_resume)(void);
    
    /**
     * @brief 应用停止回调
     * @note 在应用停止时调用，用于清理资源
     */
    void (*on_stop)(void);
    
    /**
     * @brief 应用错误处理回调
     * @note 在应用发生错误时调用
     * @param error_code 错误代码
     */
    void (*on_error)(uint32_t error_code);
    
} AppLifecycleCallbacks_t;

/* ========================= 应用信息 ========================= */
typedef struct {
    const char* name;               // 应用名称
    const char* version;            // 应用版本
    const char* description;        // 应用描述
    ProjectType_t type;             // 项目类型
    AppState_t state;               // 当前状态
    uint32_t run_time_ms;           // 运行时间（毫秒）
} AppInfo_t;

/* ========================= 传感器数据定义 ========================= */
/**
 * @brief 标准传感器数据结构
 */
typedef struct {
    float temperature;              // 温度（℃）
    float humidity;                 // 湿度（%RH）
    float co_concentration;         // CO浓度（PPM）
    float light_intensity;          // 光照强度（Lux）
    float distance;                 // 距离（cm）
    uint32_t timestamp;             // 时间戳
    bool valid;                     // 数据有效标志
} SensorData_t;

/**
 * @brief 控制命令定义（可由项目扩展）
 */
typedef struct {
    uint8_t direction;              // 方向控制 (0-停止, 1-前进, 2-后退, 3-左转, 4-右转)
    uint8_t speed;                  // 速度档位 (0-停止, 1-25%, 2-50%, 3-75%, 4-100%)
    uint8_t mode;                   // 工作模式
    bool enable;                    // 使能标志
} ControlData_t;

/* ========================= 应用框架接口 ========================= */

/**
 * @brief 初始化应用框架
 * @param callbacks 生命周期回调函数
 * @param app_info 应用信息
 * @return true: 成功, false: 失败
 */
bool AppFramework_Init(AppLifecycleCallbacks_t* callbacks, AppInfo_t* app_info);

/**
 * @brief 启动应用
 * @return true: 成功, false: 失败
 */
bool AppFramework_Start(void);

/**
 * @brief 运行应用主循环
 * @note 这个函数包含无限循环，不会返回
 */
void AppFramework_Run(void);

/**
 * @brief 暂停应用
 * @return true: 成功, false: 失败
 */
bool AppFramework_Pause(void);

/**
 * @brief 恢复应用
 * @return true: 成功, false: 失败
 */
bool AppFramework_Resume(void);

/**
 * @brief 停止应用
 * @return true: 成功, false: 失败
 */
bool AppFramework_Stop(void);

/**
 * @brief 获取应用状态
 * @return 当前应用状态
 */
AppState_t AppFramework_GetState(void);

/**
 * @brief 获取应用信息
 * @return 应用信息指针
 */
AppInfo_t* AppFramework_GetInfo(void);

/**
 * @brief 应用错误处理
 * @param error_code 错误代码
 */
void AppFramework_HandleError(uint32_t error_code);

/* ========================= 辅助功能接口 ========================= */

/**
 * @brief 获取系统运行时间（毫秒）
 * @return 运行时间
 */
uint32_t AppFramework_GetRunTime(void);

/**
 * @brief 延时函数（毫秒）
 * @param ms 延时时间
 */
void AppFramework_Delay(uint32_t ms);

/**
 * @brief 获取当前传感器数据
 * @param data 数据结构指针
 * @return true: 成功, false: 失败
 */
bool AppFramework_GetSensorData(SensorData_t* data);

/**
 * @brief 设置控制数据
 * @param data 控制数据指针
 * @return true: 成功, false: 失败
 */
bool AppFramework_SetControlData(ControlData_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __APP_FRAMEWORK_H__ */
