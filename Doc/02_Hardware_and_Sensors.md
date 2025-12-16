# 02 - 硬件与传感器说明（用途 / 协议 / 使用方法）

> 引脚速查请看 `Doc/Trash_Can_Pin_define.md`，本文聚焦为什么选这些器件、它们怎么连、协议是什么、在代码里怎么用。

## 超声波测距（HC-SR04 类）×2
- **协议/接口**：GPIO 触发 + 定时器输入捕获。Trig 产生 >10 µs 脉冲，Echo 由 TIM8 CH3/4（桶1）与 CH1/2（桶2）测量高电平宽度。
- **为什么用**：可靠、便宜，4 cm 阈值内有人靠近就开盖。
- **怎么用**：
  - 硬件：Trig1/2 接 PA9/PA11，Echo1/2 接 PA8/PA10（见引脚表）。
  - 代码：`Device/ultrasonic.c` 里 `ultrasonic_task1/2()` 触发测距；回波处理在 `HAL_TIM_IC_CaptureCallback`（`App/User_Main.c`）。距离计算 `distance = (high_time*0.034)/2`（声速约 340 m/s）。
  - 调参数：将 `distance <= 4` 的阈值改大/改小可调整灵敏度。

## 温湿度 AHT30 ×2
- **协议**：I2C（地址 0x70）。项目用两路 I2C：`hi2c1`(PB6/PB7) 与 `hi2c2`(PB10/PB11)，避免总线冲突。
- **为什么用**：单总线、精度高，适合桶内温湿度监测。
- **怎么用**：
  - 初始化：`AHT30_Init()` / `AHT302_Init()`。
  - 读取：`AHT30_Read(&temp, &hum)` / `AHT302_Read(...)`。内部发送 `{0xAC,0x33,0x00}` 触发测量，延时 75 ms 后读 6 字节再换算温湿度。
  - 显示：主循环中分别刷新 OLED1/2。

## MQ-2 烟雾传感器 ×2（模拟量）
- **协议**：ADC（DMA）读取模拟电压，连接 ADC1 CH0/CH1（PA0/PA1）。
- **为什么用**：快速检测可燃气体/烟雾浓度。
- **怎么用**：
  - 采样：`HAL_ADC_Start_DMA(&hadc1, adc_buffer, 4)`，结果在 `HAL_ADC_ConvCpltCallback` 中的 `value1`(桶1) / `value4`(桶2)。
  - 阈值：`>= 1000` 触发蜂鸣器 + 红灯。根据实际标定调整 `1000`。
  - 标定提示：预热传感器，空载记录基线，再设定合适的报警阈值。

## TCRT5000 红外检测（桶满）×2
- **协议**：ADC（DMA），连接 ADC1 CH4/CH5（PA4/PA5）。代码中用 `value2`/`value3`。
- **为什么用**：检测桶内是否有物体遮挡以判断“满”。
- **怎么用**：
  - 阈值：`value < 1000` 认为满（`full_flag1/2 = 1`），对应桶的舵机将被禁止再打开。
  - 调整：根据安装距离/背景反射调阈值，越小代表更暗/遮挡更强。
  - OLED 提示：满则显示 `Full`，否则 `Not Full`。

## 舵机（SG90/舵盘）×2
- **协议**：PWM，TIM3 CH1/CH2，周期由 CubeMX 配置。占空比决定角度。
- **为什么用**：直接驱动桶盖，控制简单。
- **怎么用**：
  - 初始化：`servo_init()` 启动 PWM。
  - 角度：`servo1_open()` 设置 CCR1=240；`servo1_close()` 设置 CCR1=150。桶2对应 CCR2=80（开）/150（关）。可根据实际行程调整比较值。
  - 逻辑：若 `full_flag` 为 1，开盖命令将被忽略，防止溢出。

## OLED 0.96" ×2（SSD1306 协议兼容）
- **协议**：软 I2C（模拟 GPIO 时序），地址 0x78（写）。
  - OLED1：PB15(SCL)/PB14(SDA)；OLED2：PB13(SCL)/PB12(SDA)。
- **为什么用**：现场直接显示温度/湿度/状态，方便调试。
- **怎么用**：
  - 初始化：`OLED_Init()` / `OLED2_Init()`。
  - 常用函数：`OLED_ShowString`、`OLED_ShowNum`、`OLED_ShowFloat`（两块屏分别有 OLED 与 OLED2 前缀）。
  - 字模：`Device/OLED_Font.c`。如需中文需额外点阵库，目前仅 ASCII。

## 蜂鸣器 ×2 与 RGB LED
- **协议**：GPIO 推挽输出。
  - 蜂鸣器：`Beep1_TurnOn/Off/Alarm`（PC4），`Beep2_*`（PC5）。
  - RGB：`Red/Green/Blue_TurnOn/Off/Twinkle`（PC1/PC2/PC3），当前代码只用了红灯做报警。
- **使用建议**：蜂鸣器为高电平有效；闪烁函数有 10 次上限防止阻塞太久。

## 语音/串口模块
- **协议**：UART2，DMA 接收 1 字节命令再回发回执。
- **为什么用**：无需按键即可开合，适配语音识别模块或上位机。
- **指令**：
  - `0x01` 开桶1，`0x02` 关桶1，`0x03` 开桶2，`0x04` 关桶2。
  - 回执：收到什么回传什么，便于上位机确认。
- **代码位置**：`HAL_UART_RxCpltCallback` in `App/User_Main.c`。

## 报警与联动策略（汇总）
- 温度任一 > 28 °C 或任一 MQ-2 ≥ 1000 → `Beep2_TurnOn()` + `Red_TurnOn()`。
- 温度正常且两路 MQ-2 < 1000 → 关闭报警。
- 桶满标志 `full_flag` 被置位时，对应舵机忽略开盖请求；OLED 显示 `Full`。

## 接线与调试小贴士
- 超声波 Echo 必须接定时器输入捕获通道（带复用），Trig 用普通输出即可。
- 两块 AHT30 走两路 I2C，布线避免过长；如果只留一块，删除另一块初始化与读数即可。
- MQ-2 与 TCRT5000 都是模拟量，ADC 采样前先 `HAL_ADCEx_Calibration_Start` 已在代码中调用。
- 语音/串口模块默认 115200 bps 8N1（CubeMX 默认），如有不同请在 `usart.c` 配置。
