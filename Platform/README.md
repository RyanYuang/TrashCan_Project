# STM32 IoT 平台使用指南

## 📋 目录

1. [平台简介](#平台简介)
2. [平台架构](#平台架构)
3. [快速开始](#快速开始)
4. [详细说明](#详细说明)
5. [API参考](#api参考)
6. [项目示例](#项目示例)
7. [常见问题](#常见问题)

---

## 🎯 平台简介

这是一个基于STM32F103的IoT应用开发平台，提供了统一的设备管理、应用框架和配置系统，让你可以快速开发各种物联网项目。

### 主要特性

- ✅ **统一的设备管理** - 通过设备管理器统一管理所有硬件设备
- ✅ **应用框架** - 提供生命周期管理，简化应用开发
- ✅ **配置分层** - 平台配置和项目配置分离，便于维护
- ✅ **模块化设计** - 各功能模块解耦，易于扩展
- ✅ **项目模板** - 提供完整的项目模板，快速启动新项目

### 支持的设备

- **显示器**: OLED (SSD1306)
- **传感器**: AHT30(温湿度)、GY302/BH1750(光照)、MQ-2(气体)、超声波
- **执行器**: TB6612(电机驱动)、RGB LED、蜂鸣器
- **通信**: LoRa (JC278A)、UART协议

---

## 🏗️ 平台架构

```
TrashCan_Project/
├── Platform/                    # 平台核心
│   ├── Config/                  # 配置系统
│   │   ├── platform_config.h    # 平台基础配置
│   │   └── project_config.h     # 项目配置（可覆盖平台配置）
│   ├── DeviceManager/           # 设备管理
│   │   ├── device_manager.h/.c  # 设备管理器
│   │   └── device_config.h      # 设备配置接口
│   ├── AppFramework/            # 应用框架
│   │   └── app_framework.h/.c   # 应用生命周期管理
│   └── platform.h               # 平台总头文件
│
├── Projects/                    # 项目实现
│   ├── Template/                # 项目模板
│   │   ├── project_template.c   # 模板实现
│   │   └── project_template.h   # 模板头文件
│   └── TrashCan/                # 垃圾桶项目示例
│       ├── trash_can_app.c      # 项目实现
│       └── trash_can_app.h      # 项目头文件
│
├── Device/                      # 设备驱动
├── Driver/                      # 底层驱动
├── Core/                        # STM32 HAL库
└── App/                         # 原有应用代码（兼容）
```

### 架构分层

```
┌─────────────────────────────────────┐
│        应用层 (Projects)             │  ← 项目特定逻辑
├─────────────────────────────────────┤
│       应用框架 (AppFramework)        │  ← 生命周期管理
├─────────────────────────────────────┤
│      设备管理器 (DeviceManager)      │  ← 设备统一管理
├─────────────────────────────────────┤
│      设备驱动层 (Device/Driver)      │  ← 硬件驱动
├─────────────────────────────────────┤
│         HAL库 (Core/Drivers)         │  ← STM32 HAL
└─────────────────────────────────────┘
```

---

## 🚀 快速开始

### 方式一：使用模板创建新项目

1. **复制项目模板**
   ```bash
   cp -r Projects/Template Projects/MyProject
   cd Projects/MyProject
   ```

2. **修改项目配置**
   
   在 `project_template.c` 中修改：
   ```c
   #define MY_PROJECT_NAME         "MyProject"
   #define MY_PROJECT_VERSION      "1.0.0"
   #define MY_PROJECT_DESCRIPTION  "My Project Description"
   #define MY_PROJECT_TYPE         PROJECT_TYPE_CUSTOM
   ```

3. **初始化设备**
   
   在 `InitDevices()` 函数中添加设备初始化代码：
   ```c
   static bool InitDevices(void)
   {
       // OLED 初始化示例
       Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
       Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};
       OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
       OLED_Init(&oled1);
       
       return true;
   }
   ```

4. **实现主逻辑**
   
   在 `MyProject_OnLoop()` 中实现：
   ```c
   void MyProject_OnLoop(void)
   {
       // 读取传感器
       // 处理数据
       // 控制执行器
   }
   ```

5. **调用应用**
   
   在 `main.c` 的 `User_Main()` 中：
   ```c
   #include "project_template.h"
   
   void User_Main(void)
   {
       MyProject_App_Main();  // 调用你的应用
   }
   ```

### 方式二：参考示例项目

查看 `Projects/TrashCan/` 目录下的完整实现示例。

---

## 📖 详细说明

### 1. 配置系统

#### 平台配置 (platform_config.h)

定义平台级别的基础配置，所有项目共享：

```c
// 调试使能
#define PLATFORM_DEBUG_ENABLE       1

// 设备支持
#define PLATFORM_DEVICE_OLED        1
#define PLATFORM_DEVICE_AHT30       1

// 数据上报周期
#define PLATFORM_DATA_REPORT_PERIOD_MS  2000
```

#### 项目配置 (project_config.h)

每个项目可以覆盖平台配置：

```c
// 项目信息
#define PROJECT_NAME                "MyProject"
#define PROJECT_VERSION             "1.0.0"

// 覆盖平台配置
#define PROJECT_DATA_REPORT_PERIOD_MS   1000  // 使用1秒上报
```

### 2. 设备管理器

#### 设备注册

```c
// 1. 定义设备描述符
DeviceDescriptor_t my_device = {
    .info = {
        .id = DEVICE_ID_OLED,
        .name = "OLED Display",
        .type = DEVICE_TYPE_DISPLAY,
        .status = DEVICE_STATUS_UNINIT,
        .handle = &oled_handle,
        .config = &oled_config
    },
    .ops = {
        .init = OLED_Init_Wrapper,
        .read = NULL,
        .write = OLED_Write_Wrapper,
        .control = OLED_Control_Wrapper
    }
};

// 2. 注册设备
DeviceManager_Register(&my_device);

// 3. 初始化设备
DeviceManager_InitDevice(DEVICE_ID_OLED);
```

#### 设备操作

```c
// 读取设备数据
float temperature;
DeviceManager_ReadDevice(DEVICE_ID_AHT30, &temperature);

// 写入设备数据
uint8_t display_text[] = "Hello";
DeviceManager_WriteDevice(DEVICE_ID_OLED, display_text);

// 控制设备
uint32_t brightness = 128;
DeviceManager_ControlDevice(DEVICE_ID_OLED, CMD_SET_BRIGHTNESS, &brightness);
```

### 3. 应用框架

#### 生命周期回调

```c
AppLifecycleCallbacks_t callbacks = {
    .on_init = MyApp_OnInit,      // 初始化（调用一次）
    .on_start = MyApp_OnStart,    // 启动（初始化成功后调用）
    .on_loop = MyApp_OnLoop,      // 主循环（周期性调用）
    .on_pause = MyApp_OnPause,    // 暂停（可选）
    .on_resume = MyApp_OnResume,  // 恢复（可选）
    .on_stop = MyApp_OnStop,      // 停止（清理资源）
    .on_error = MyApp_OnError     // 错误处理
};
```

#### 应用状态

```c
// 获取当前状态
AppState_t state = AppFramework_GetState();

// 状态包括：
// - APP_STATE_UNINIT    : 未初始化
// - APP_STATE_INIT      : 初始化中
// - APP_STATE_RUNNING   : 运行中
// - APP_STATE_PAUSED    : 暂停
// - APP_STATE_ERROR     : 错误
// - APP_STATE_SHUTDOWN  : 关闭
```

---

## 📚 API参考

### 平台初始化

```c
bool Platform_Init(void);
void Platform_PrintInfo(void);
```

### 设备管理器

```c
bool DeviceManager_Init(void);
bool DeviceManager_Register(DeviceDescriptor_t* descriptor);
bool DeviceManager_InitDevice(DeviceID_t id);
bool DeviceManager_ReadDevice(DeviceID_t id, void* data);
bool DeviceManager_WriteDevice(DeviceID_t id, void* data);
bool DeviceManager_ControlDevice(DeviceID_t id, uint32_t cmd, void* param);
DeviceStatus_t DeviceManager_GetDeviceStatus(DeviceID_t id);
```

### 应用框架

```c
bool AppFramework_Init(AppLifecycleCallbacks_t* callbacks, AppInfo_t* app_info);
bool AppFramework_Start(void);
void AppFramework_Run(void);
bool AppFramework_Pause(void);
bool AppFramework_Resume(void);
bool AppFramework_Stop(void);
AppState_t AppFramework_GetState(void);
uint32_t AppFramework_GetRunTime(void);
```

---

## 💡 项目示例

### 示例1：简单的传感器读取

```c
bool MyApp_OnInit(void)
{
    Platform_Init();
    
    // 初始化温湿度传感器
    Pin_Struct sda = {GPIOC, GPIO_PIN_0};
    Pin_Struct scl = {GPIOC, GPIO_PIN_13};
    AHT30_Device_Create(&aht30, &sda, &scl, 0x38);
    AHT30_Init(&aht30);
    
    return true;
}

void MyApp_OnLoop(void)
{
    float temperature, humidity;
    
    // 读取数据
    AHT30_Read_Data(&aht30, &temperature, &humidity);
    
    // 打印数据
    printf("Temp: %.1f°C, Humidity: %.1f%%\r\n", temperature, humidity);
    
    // 延时
    HAL_Delay(1000);
}
```

### 示例2：带显示的完整应用

参考 `Projects/TrashCan/trash_can_app.c`

---

## ❓ 常见问题

### Q1: 如何添加新的设备类型？

在 `device_manager.h` 中添加设备ID：
```c
typedef enum {
    // ... 现有设备
    DEVICE_ID_MY_NEW_DEVICE,
    DEVICE_ID_MAX
} DeviceID_t;
```

### Q2: 如何修改数据上报周期？

在项目配置中覆盖：
```c
#define PROJECT_DATA_REPORT_PERIOD_MS   1000  // 改为1秒
```

### Q3: 如何禁用某个设备？

在项目配置中：
```c
#define PROJECT_USE_ULTRASONIC  0  // 禁用超声波传感器
```

### Q4: 如何切换项目？

修改 `main.c` 中的调用：
```c
void User_Main(void)
{
    // TrashCan_App_Main();     // 注释掉旧项目
    MyNewProject_App_Main();    // 启用新项目
}
```

### Q5: 平台占用多少资源？

- Flash: 约 5-8KB (框架代码)
- RAM: 约 1-2KB (设备管理器和应用框架)
- 实际占用取决于使用的设备和功能

---

## 📝 开发建议

1. **先看示例** - 从 TrashCan 项目开始了解平台用法
2. **使用模板** - 基于模板创建新项目，减少重复代码
3. **分层设计** - 应用逻辑与设备驱动分离
4. **统一管理** - 使用设备管理器管理所有设备
5. **配置驱动** - 通过配置文件控制功能开关

---

## 🔄 版本历史

- **v1.0.0** (2026-02-18)
  - 初始版本发布
  - 设备管理器
  - 应用框架
  - 配置系统
  - 项目模板

---

## 📧 支持

如有问题请参考：
- 代码注释
- 示例项目
- 本文档

---

**Happy Coding! 🎉**
