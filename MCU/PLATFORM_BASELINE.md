# STM32 IoT 平台 - 基线分支

> **版本**: 1.0.0  
> **日期**: 2026-02-18  
> **分支**: master (基线)

---

## 📌 关于此基线

这是一个**平台化的基线分支**，为后续所有项目提供统一的开发基础。基于这个基线，你可以快速开发各种物联网应用项目，无需从零开始。

### 🎯 基线包含什么？

✅ **完整的平台框架**
- 配置系统（平台配置 + 项目配置）
- 设备管理器（统一设备接口）
- 应用框架（生命周期管理）

✅ **设备驱动库**
- OLED显示器、温湿度传感器、光照传感器
- 气体传感器、电机驱动、RGB LED
- 蜂鸣器、超声波、LoRa通信

✅ **项目模板**
- 完整的项目代码模板
- 详细的注释和使用说明
- 快速启动新项目

✅ **示例项目**
- TrashCan (垃圾桶) - 完整实现示例
- 演示平台的各种功能使用

✅ **完整文档**
- 平台使用指南
- 架构设计文档
- API参考手册

---

## 🚀 快速开始

### 1️⃣ 创建新项目

```bash
# 进入项目目录
cd TrashCan_Project/Projects

# 复制模板
cp -r Template MyNewProject

# 编辑项目
cd MyNewProject
# 修改 project_template.c 中的配置
```

### 2️⃣ 修改项目配置

```c
// 在 project_template.c 中修改
#define MY_PROJECT_NAME         "MyNewProject"
#define MY_PROJECT_VERSION      "1.0.0"
#define MY_PROJECT_DESCRIPTION  "My awesome project"
#define MY_PROJECT_TYPE         PROJECT_TYPE_CUSTOM
```

### 3️⃣ 实现项目逻辑

```c
// 初始化设备
bool MyProject_OnInit(void) {
    Platform_Init();
    InitDevices();  // 初始化你需要的设备
    return true;
}

// 主循环逻辑
void MyProject_OnLoop(void) {
    // 你的业务逻辑
    ReadSensors();
    ProcessData();
    UpdateDisplay();
}
```

### 4️⃣ 在main.c中调用

```c
// App/User_Main.c 或 Core/Src/main.c
#include "project_template.h"

void User_Main(void) {
    MyProject_App_Main();  // 启动你的应用
}
```

---

## 📁 目录结构

```
TrashCan_Project/
│
├── Platform/              ⭐ 平台核心（基线的核心）
│   ├── Config/           - 配置系统
│   ├── DeviceManager/    - 设备管理器
│   ├── AppFramework/     - 应用框架
│   ├── platform.h        - 平台总头文件
│   ├── README.md         - 使用指南
│   └── ARCHITECTURE.md   - 架构文档
│
├── Projects/              ⭐ 项目实现（基于基线开发）
│   ├── Template/         - 项目模板（复制此模板创建新项目）
│   └── TrashCan/         - 示例项目（参考实现）
│
├── Device/               - 设备驱动（已有设备）
├── Driver/               - 底层驱动
├── Core/                 - STM32 HAL库
└── App/                  - 应用代码（兼容旧代码）
```

---

## 📖 文档索引

| 文档 | 位置 | 说明 |
|------|------|------|
| 平台使用指南 | `Platform/README.md` | 如何使用平台开发项目 |
| 架构设计文档 | `Platform/ARCHITECTURE.md` | 平台的设计理念和架构 |
| 项目模板 | `Projects/Template/` | 新项目的起点 |
| 示例项目 | `Projects/TrashCan/` | 完整的实现示例 |

---

## 🌟 平台特性

### 统一的设备管理

```c
// 所有设备通过统一接口访问
DeviceManager_InitDevice(DEVICE_ID_OLED);
DeviceManager_ReadDevice(DEVICE_ID_AHT30, &data);
DeviceManager_WriteDevice(DEVICE_ID_OLED, text);
```

### 标准化的应用框架

```c
// 标准的生命周期管理
AppLifecycleCallbacks_t callbacks = {
    .on_init = MyApp_Init,
    .on_start = MyApp_Start,
    .on_loop = MyApp_Loop,
    .on_stop = MyApp_Stop
};
```

### 灵活的配置系统

```c
// 平台配置（所有项目共享）
#define PLATFORM_DEBUG_ENABLE       1
#define PLATFORM_DEVICE_OLED        1

// 项目配置（可覆盖平台配置）
#define PROJECT_DATA_REPORT_PERIOD_MS   1000
```

---

## 🎯 基于基线开发新项目

### 方式一：使用模板（推荐）

1. 复制 `Projects/Template/` 
2. 修改项目配置
3. 实现业务逻辑
4. 编译下载

⏱️ **预计时间**: 10-30分钟创建一个新项目

### 方式二：参考示例

查看 `Projects/TrashCan/` 了解完整实现流程

---

## 💡 使用建议

### ✅ 推荐做法

- ✅ 使用模板创建新项目
- ✅ 通过配置文件控制功能
- ✅ 使用设备管理器访问设备
- ✅ 遵循应用框架的生命周期
- ✅ 项目代码放在 `Projects/YourProject/`

### ❌ 不推荐做法

- ❌ 修改平台核心代码（除非必要）
- ❌ 直接操作硬件寄存器
- ❌ 绕过设备管理器直接调用驱动
- ❌ 在主循环中阻塞太久

---

## 🔄 项目示例

### 已实现项目

1. **TrashCan** - 智能垃圾桶
   - 温湿度监测
   - 气体检测
   - 光照检测
   - 远程控制
   - OLED显示

### 可以快速开发的项目类型

- 🚗 环境监测车 (EnvCar)
- 🏠 智能家居 (SmartHome)
- 🌡️ 气象站 (WeatherStation)
- 🚪 门禁系统 (AccessControl)
- 🤖 巡线小车 (LineFollower)
- 💡 智能照明 (SmartLight)
- ... 更多可能

---

## 🛠️ 开发工具链

- **IDE**: STM32CubeIDE / Keil / IAR
- **MCU**: STM32F103RCT6
- **调试**: ST-Link / DAP-Link
- **串口**: 115200 bps (默认)

---

## 📊 平台优势

| 对比项 | 传统开发 | 平台化开发 |
|--------|----------|------------|
| 新项目启动 | 数小时-数天 | 10-30分钟 |
| 代码复用 | 低（<20%） | 高（>70%） |
| 学习曲线 | 陡峭 | 平缓 |
| 代码一致性 | 差 | 优秀 |
| 维护成本 | 高 | 低 |
| 扩展性 | 差 | 好 |

---

## 🔧 自定义和扩展

### 添加新设备

1. 在 `Device/` 下编写驱动
2. 在 `device_manager.h` 中添加设备ID
3. 实现设备操作接口
4. 在项目中注册和使用

### 修改平台配置

编辑 `Platform/Config/platform_config.h`

### 添加新功能模块

在 `Platform/` 下创建新目录

---

## 📞 技术支持

- 📖 查看文档: `Platform/README.md`
- 🔍 参考示例: `Projects/TrashCan/`
- 📝 查看注释: 代码中有详细注释
- 🎯 使用模板: `Projects/Template/`

---

## 🎓 学习路径

1. **了解架构** → 阅读 `Platform/ARCHITECTURE.md`
2. **学习使用** → 阅读 `Platform/README.md`
3. **查看示例** → 研究 `Projects/TrashCan/`
4. **动手实践** → 使用模板创建项目
5. **深入学习** → 阅读平台源码

---

## ⚠️ 重要提示

### 关于基线分支

- 🔒 **不要直接在基线分支开发具体项目**
- 🌿 **为每个新项目创建独立分支**
- 🔄 **定期从基线分支合并更新**
- 📝 **记录对基线的修改**

### 分支管理建议

```bash
# 创建新项目分支
git checkout -b project/MyNewProject

# 从基线更新
git merge master
```

---

## 🎉 开始你的第一个项目

```bash
# 1. 复制模板
cd TrashCan_Project/Projects
cp -r Template MyFirstProject

# 2. 开始编辑
cd MyFirstProject
# 修改 project_template.c

# 3. 编译运行
# 在IDE中编译并下载到MCU

# 4. 查看效果
# 通过串口查看调试输出
```

---

**让我们基于这个平台，快速开发更多有趣的物联网项目！** 🚀

---

© 2026 STM32 IoT Platform Team
