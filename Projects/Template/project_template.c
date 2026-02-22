/**
 ******************************************************************************
 * @file    project_template.c
 * @brief   项目模板 - 快速创建新项目的起点
 * @note    复制此模板并修改即可创建新项目
 ******************************************************************************
 */

#include "project_template.h"
#include "platform.h"
#include "Devices.h"
#include "Drivers.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/* ========================= 配置区域 - 请根据项目需求修改 ========================= */

// 项目名称（请修改）
#define MY_PROJECT_NAME         "MyProject"
#define MY_PROJECT_VERSION      "1.0.0"
#define MY_PROJECT_DESCRIPTION  "My Custom Project Description"
#define MY_PROJECT_TYPE         PROJECT_TYPE_CUSTOM

/* ========================= 私有变量 ========================= */
// TODO: 在这里定义项目需要的设备句柄、传感器数据等变量

/* ========================= 私有函数声明 ========================= */
static bool InitDevices(void);
static void ProcessData(void);

/* ========================= 应用生命周期实现 ========================= */

/**
 * @brief 应用初始化
 * @note 在这里初始化平台、设备和项目特定资源
 */
bool MyProject_OnInit(void)
{
    printf("[%s] Application initializing...\r\n", MY_PROJECT_NAME);
    
    // 初始化平台
    if(!Platform_Init()) {
        printf("[%s] Platform initialization failed\r\n", MY_PROJECT_NAME);
        return false;
    }
    
    // 打印平台信息
    Platform_PrintInfo();
    
    // TODO: 在这里添加项目特定的初始化代码
    // 例如：初始化协议、配置模块等
    
    // 初始化设备
    if(!InitDevices()) {
        printf("[%s] Device initialization failed\r\n", MY_PROJECT_NAME);
        return false;
    }
    
    // TODO: 启动必要的定时器、中断等
    
    printf("[%s] Application initialized successfully\r\n", MY_PROJECT_NAME);
    return true;
}

/**
 * @brief 应用启动
 * @note 初始化完成后调用，可以在这里做一些启动提示
 */
void MyProject_OnStart(void)
{
    printf("[%s] Application started\r\n", MY_PROJECT_NAME);
    
    // TODO: 在这里添加启动时的操作
    // 例如：显示欢迎信息、LED指示等
}

/**
 * @brief 应用主循环
 * @note 这是应用的核心逻辑，周期性执行
 */
void MyProject_OnLoop(void)
{
    // TODO: 在这里实现项目的主要逻辑
    
    // 示例：数据处理
    ProcessData();
    
    // TODO: 添加更多功能
    // - 读取传感器
    // - 处理通信数据
    // - 控制执行器
    // - 显示更新
    // - 报警判断
}

/**
 * @brief 应用暂停
 * @note 可选实现，当应用需要暂停时调用
 */
void MyProject_OnPause(void)
{
    printf("[%s] Application paused\r\n", MY_PROJECT_NAME);
    
    // TODO: 在这里添加暂停时的操作
    // 例如：停止数据采集、保存状态等
}

/**
 * @brief 应用恢复
 * @note 可选实现，当应用从暂停恢复时调用
 */
void MyProject_OnResume(void)
{
    printf("[%s] Application resumed\r\n", MY_PROJECT_NAME);
    
    // TODO: 在这里添加恢复时的操作
    // 例如：恢复数据采集、恢复状态等
}

/**
 * @brief 应用停止
 * @note 应用停止时调用，用于资源清理
 */
void MyProject_OnStop(void)
{
    printf("[%s] Application stopped\r\n", MY_PROJECT_NAME);
    
    // TODO: 在这里清理资源
    // 例如：停止电机、关闭LED、保存数据等
}

/**
 * @brief 应用错误处理
 * @note 当发生错误时调用
 */
void MyProject_OnError(uint32_t error_code)
{
    printf("[%s] Error occurred: %lu\r\n", MY_PROJECT_NAME, error_code);
    
    // TODO: 在这里处理错误
    // 例如：错误提示、安全停止、错误恢复等
}

/* ========================= 私有函数实现 ========================= */

/**
 * @brief 初始化所有设备
 * @note 在这里初始化项目需要的所有硬件设备
 */
static bool InitDevices(void)
{
    bool ret = true;
    
    // TODO: 在这里初始化设备
    // 示例：
    /*
    // OLED 初始化
    Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
    Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};
    OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
    if(OLED_Device_Detection(&oled1) == OLED_Status_OK) {
        printf("[%s] OLED detected\r\n", MY_PROJECT_NAME);
        OLED_Init(&oled1);
    } else {
        printf("[%s] OLED not detected\r\n", MY_PROJECT_NAME);
        ret = false;
    }
    */
    
    return ret;
}

/**
 * @brief 数据处理
 * @note 在这里处理传感器数据、通信数据等
 */
static void ProcessData(void)
{
    // TODO: 在这里实现数据处理逻辑
    // 示例：
    // - 读取传感器数据
    // - 数据滤波
    // - 数据上报
    // - 显示更新
}

/* ========================= 应用入口 ========================= */

/**
 * @brief 项目应用入口函数
 * @note 在main.c的User_Main()中调用此函数
 */
void MyProject_App_Main(void)
{
    // 定义应用生命周期回调
    AppLifecycleCallbacks_t callbacks = {
        .on_init = MyProject_OnInit,
        .on_start = MyProject_OnStart,
        .on_loop = MyProject_OnLoop,
        .on_pause = MyProject_OnPause,      // 可选
        .on_resume = MyProject_OnResume,    // 可选
        .on_stop = MyProject_OnStop,
        .on_error = MyProject_OnError
    };
    
    // 定义应用信息
    AppInfo_t app_info = {
        .name = MY_PROJECT_NAME,
        .version = MY_PROJECT_VERSION,
        .description = MY_PROJECT_DESCRIPTION,
        .type = MY_PROJECT_TYPE,
        .state = APP_STATE_UNINIT,
        .run_time_ms = 0
    };
    
    // 初始化应用框架
    if(!AppFramework_Init(&callbacks, &app_info)) {
        printf("[%s] Failed to initialize app framework\r\n", MY_PROJECT_NAME);
        return;
    }
    
    // 启动应用
    if(!AppFramework_Start()) {
        printf("[%s] Failed to start application\r\n", MY_PROJECT_NAME);
        return;
    }
    
    // 运行应用主循环（此函数不会返回）
    AppFramework_Run();
}

/* ========================= 使用说明 ========================= 
 * 
 * 1. 复制此模板文件到 Projects/YourProjectName/ 目录
 * 2. 重命名文件为 your_project_app.c
 * 3. 修改配置区域的项目信息
 * 4. 在InitDevices()中初始化需要的设备
 * 5. 在MyProject_OnLoop()中实现主要逻辑
 * 6. 在main.c的User_Main()中调用 MyProject_App_Main()
 * 7. 编译并下载到MCU
 * 
 * 提示：
 * - 所有TODO标记处都是需要修改的地方
 * - 可以参考TrashCan项目的实现
 * - 使用Platform提供的设备管理器统一管理设备
 * - 利用应用框架的生命周期管理应用状态
 * 
 ================================================================= */
