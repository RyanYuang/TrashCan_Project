# Trash Can 项目文档索引

面向 STM32 新手的小白文档，按循序渐进的顺序阅读即可带你把代码跑起来、理解硬件与协议。

## 阅读顺序与目录
- `Doc/01_Project_Overview.md`：项目目标、系统分区、模块清单与整体工作流程。
- `Doc/02_Hardware_and_Sensors.md`：每个传感器/执行器的用途、通信协议、管脚、阈值与使用要点。
- `Doc/03_Firmware_Flow.md`：主循环与中断/回调的执行顺序，关键参数如何修改。
- `Doc/04_STM32_Basics.md`：从零认识 STM32F103、HAL 库、CubeMX 生成工程与常用外设基础。
- `Doc/Trash_Can_Pin_define.md`：全部引脚速查表（原有文件，保持不动）。

## 5 分钟快速上手
1) 先读 `01_Project_Overview.md` 了解目标与硬件组成，再翻 `02_Hardware_and_Sensors.md` 看每个器件怎么连、为什么选它。  
2) 打开 `TrashCan_Project.ioc` 用 STM32CubeIDE 生成/检查工程配置；编译并用 ST-Link 下载。  
3) 跟着 `03_Firmware_Flow.md` 对照 `App/User_Main.c` 理解主循环和回调，想改阈值/逻辑直接按文档中的指引修改。  
4) 如果从未碰过 STM32，先补 `04_STM32_Basics.md` 里的概念，再回来看代码会更顺畅。

## 工程定位
- 工程入口：`App/User_Main.c`
- 设备驱动：`Device/` 目录（OLED、AHT30、超声波、舵机、蜂鸣器、RGB 等）
- 生成配置：`TrashCan_Project.ioc`（CubeMX 工程）
