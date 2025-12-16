# 04 - STM32 基础快速入门（给小白）

## 你需要知道的最小概念
- **MCU 是什么**：一颗带 CPU、Flash、RAM、外设的芯片。这里用 STM32F103RCT6（ARM Cortex-M3，72 MHz）。
- **HAL 库**：ST 官方的硬件抽象层，`HAL_GPIO_WritePin`、`HAL_I2C_Master_Transmit` 等就是它提供的。CubeMX 自动生成底层初始化代码。
- **CubeMX / CubeIDE**：图形化配置时钟、引脚、外设并生成工程（`TrashCan_Project.ioc` 就是配置文件）。CubeIDE 集成编译/下载/调试。
- **中断 & 回调**：硬件事件（定时器捕获、ADC 采样完成、串口收发等）触发中断，HAL 会调用对应的回调函数，项目逻辑主要写在这些回调里。
- **DMA**：直接存取存储器，省掉 CPU 搬运数据。这里 ADC 和 UART 都用 DMA 提高效率。

## 时钟与外设速查
- 系统时钟：典型配置为 72 MHz（HSE 晶振倍频）。若时钟异常，PWM/串口/延时都会偏差。
- 关键外设：
  - TIM3：PWM 驱动舵机。
  - TIM8：输入捕获测量超声波 Echo。
  - ADC1：采集 MQ-2 与 TCRT5000（DMA 模式）。
  - I2C1 / I2C2：两路 AHT30。
  - GPIO + 软 I2C：两块 OLED。
  - USART2：语音/串口命令。

## 用 CubeIDE 重新生成/编译
1. 打开 `TrashCan_Project.ioc`（双击或用 CubeIDE 导入）。
2. 检查引脚映射是否与你的硬件一致（参考 `Doc/Trash_Can_Pin_define.md`）。如需改引脚，CubeMX 会同步更新 `Core/Src/*` 和 `Core/Inc/*`。
3. 生成代码（`Project -> Generate Code`），再点击编译按钮（小锤子）。
4. 用 ST-Link 连接开发板，点击下载（箭头）烧录。
5. 进入调试模式（小虫子图标），可以在 `App/User_Main.c`、`Device/*` 设置断点单步调试。

## 如何阅读/改动 HAL 代码
- 初始化代码在 `Core/Src/*.c`，由 CubeMX 生成；常规不手改，若要改寄存器配置尽量通过 CubeMX。
- 业务逻辑放在 `App/` 与 `Device/`。例如要改舵机角度，只改 `Device/servo.c` 的比较值即可。
- 若想更深入控制（比如提高响应速度），可以：
  - 把 `HAL_GPIO_WritePin` 换成直接操作寄存器。
  - 把软 I2C 换成硬件 I2C（需要在 CubeMX 重新配置并改驱动）。

## 小白常见疑问
- **为什么有那么多 `MX_` 函数？** 这是 CubeMX 生成的初始化入口，比如 `MX_GPIO_Init()`。
- **我只想跑一块 AHT30/一组超声波可以吗？** 可以。删掉对应的初始化和调用，保留引脚悬空或解除焊接即可。
- **延时准确吗？** `HAL_Delay` 依赖 SysTick，系统时钟正常就可靠；超声波测距用了定时器捕获，精度足够开盖。
- **想用 Keil/其他 IDE？** 也行，导入生成的源码与工程配置（`.ioc` 仍可在 CubeMX 打开），保证编译器选用 `arm-none-eabi-gcc` 或相同的 MCU 包即可。
