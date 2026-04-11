/**
 ******************************************************************************
 * @file    app_framework.c
 * @brief   应用框架实现
 ******************************************************************************
 */

#include "app_framework.h"
#include "device_manager.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

/* ========================= 私有变量 ========================= */
static AppLifecycleCallbacks_t g_app_callbacks;
static AppInfo_t g_app_info;
static bool g_framework_initialized = false;

/* ========================= 私有函数声明 ========================= */
static void AppFramework_StateTransition(AppState_t new_state);

/* ========================= 框架实现 ========================= */

/**
 * @brief 初始化应用框架
 */
bool AppFramework_Init(AppLifecycleCallbacks_t* callbacks, AppInfo_t* app_info)
{
    if(g_framework_initialized) {
        return true;
    }
    
    if(callbacks == NULL || app_info == NULL) {
        return false;
    }
    
    // 复制回调函数和应用信息
    memcpy(&g_app_callbacks, callbacks, sizeof(AppLifecycleCallbacks_t));
    memcpy(&g_app_info, app_info, sizeof(AppInfo_t));
    
    // 初始化状态
    g_app_info.state = APP_STATE_UNINIT;
    g_app_info.run_time_ms = 0;
    
    g_framework_initialized = true;
    
#if PLATFORM_DEBUG_ENABLE
    printf("[AppFramework] Framework initialized\r\n");
    printf("[AppFramework] App: %s v%s\r\n", g_app_info.name, g_app_info.version);
#endif
    
    return true;
}

/**
 * @brief 启动应用
 */
bool AppFramework_Start(void)
{
    if(!g_framework_initialized) {
        return false;
    }
    
    if(g_app_info.state != APP_STATE_UNINIT && g_app_info.state != APP_STATE_SHUTDOWN) {
        return false;
    }
    
    // 切换到初始化状态
    AppFramework_StateTransition(APP_STATE_INIT);
    
    // 调用初始化回调
    if(g_app_callbacks.on_init != NULL) {
        if(!g_app_callbacks.on_init()) {
#if PLATFORM_DEBUG_ENABLE
            printf("[AppFramework] Application initialization failed\r\n");
#endif
            AppFramework_StateTransition(APP_STATE_ERROR);
            return false;
        }
    }
    
    // 切换到运行状态
    AppFramework_StateTransition(APP_STATE_RUNNING);
    
    // 调用启动回调
    if(g_app_callbacks.on_start != NULL) {
        g_app_callbacks.on_start();
    }
    
#if PLATFORM_DEBUG_ENABLE
    printf("[AppFramework] Application started\r\n");
#endif
    
    return true;
}

/**
 * @brief 运行应用主循环
 */
void AppFramework_Run(void)
{
    if(!g_framework_initialized) {
        return;
    }
    
    // 主循环
    while(1) {
        // 检查状态
        if(g_app_info.state == APP_STATE_RUNNING) {
            // 调用循环回调
            if(g_app_callbacks.on_loop != NULL) {
                g_app_callbacks.on_loop();
            }
        } else if(g_app_info.state == APP_STATE_ERROR) {
            // 错误状态，停止循环
            if(g_app_callbacks.on_error != NULL) {
                g_app_callbacks.on_error(0);
            }
            break;
        } else if(g_app_info.state == APP_STATE_SHUTDOWN) {
            // 关闭状态，退出循环
            break;
        }
        // PAUSED状态下不调用on_loop，但继续循环等待状态改变
        
        // 短延时
        HAL_Delay(PLATFORM_MAIN_LOOP_DELAY_MS);
    }
}

/**
 * @brief 暂停应用
 */
bool AppFramework_Pause(void)
{
    if(g_app_info.state != APP_STATE_RUNNING) {
        return false;
    }
    
    AppFramework_StateTransition(APP_STATE_PAUSED);
    
    if(g_app_callbacks.on_pause != NULL) {
        g_app_callbacks.on_pause();
    }
    
    return true;
}

/**
 * @brief 恢复应用
 */
bool AppFramework_Resume(void)
{
    if(g_app_info.state != APP_STATE_PAUSED) {
        return false;
    }
    
    AppFramework_StateTransition(APP_STATE_RUNNING);
    
    if(g_app_callbacks.on_resume != NULL) {
        g_app_callbacks.on_resume();
    }
    
    return true;
}

/**
 * @brief 停止应用
 */
bool AppFramework_Stop(void)
{
    if(g_app_info.state == APP_STATE_UNINIT || g_app_info.state == APP_STATE_SHUTDOWN) {
        return false;
    }
    
    if(g_app_callbacks.on_stop != NULL) {
        g_app_callbacks.on_stop();
    }
    
    AppFramework_StateTransition(APP_STATE_SHUTDOWN);
    
    return true;
}

/**
 * @brief 获取应用状态
 */
AppState_t AppFramework_GetState(void)
{
    return g_app_info.state;
}

/**
 * @brief 获取应用信息
 */
AppInfo_t* AppFramework_GetInfo(void)
{
    return &g_app_info;
}

/**
 * @brief 应用错误处理
 */
void AppFramework_HandleError(uint32_t error_code)
{
    AppFramework_StateTransition(APP_STATE_ERROR);
    
    if(g_app_callbacks.on_error != NULL) {
        g_app_callbacks.on_error(error_code);
    }
}

/**
 * @brief 状态转换
 */
static void AppFramework_StateTransition(AppState_t new_state)
{
    AppState_t old_state = g_app_info.state;
    g_app_info.state = new_state;
    
#if PLATFORM_DEBUG_ENABLE
    const char* state_names[] = {
        "UNINIT", "INIT", "RUNNING", "PAUSED", "ERROR", "SHUTDOWN"
    };
    printf("[AppFramework] State: %s -> %s\r\n", 
           state_names[old_state], state_names[new_state]);
#endif
}

/**
 * @brief 获取系统运行时间
 */
uint32_t AppFramework_GetRunTime(void)
{
    return HAL_GetTick();
}

/**
 * @brief 延时函数
 */
void AppFramework_Delay(uint32_t ms)
{
    HAL_Delay(ms);
}

/**
 * @brief 获取当前传感器数据（需要由具体项目实现）
 */
__weak bool AppFramework_GetSensorData(SensorData_t* data)
{
    // 默认实现，返回空数据
    if(data != NULL) {
        memset(data, 0, sizeof(SensorData_t));
        data->valid = false;
    }
    return false;
}

/**
 * @brief 设置控制数据（需要由具体项目实现）
 */
__weak bool AppFramework_SetControlData(ControlData_t* data)
{
    // 默认实现，不执行任何操作
    return false;
}
