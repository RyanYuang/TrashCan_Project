# 03 - 固件执行流程与关键代码

入口文件：`App/User_Main.c`。这一节按照“初始化 → 主循环 → 回调”梳理代码，方便小白对号入座。

## 初始化（`User_Main()` 开头）
- `servo_init()`：开启 TIM3 CH1/CH2 PWM，舵机默认关。
- `OLED_Init()` / `OLED2_Init()`：软 I2C 初始化，两块屏幕清屏。
- `AHT30_Init()` / `AHT302_Init()`：两路温湿度上电配置。
- `ultrasonic_init()`：开启 TIM8 输入捕获（带中断），为双路超声波做准备。
- `HAL_ADCEx_Calibration_Start(&hadc1)`：校准 ADC1，确保后续 DMA 数据准确。

## 主循环（`while(1)`）
1. `HAL_ADC_Start_DMA(&hadc1, adc_buffer, 4)`：启动 4 路模拟采样（MQ-2×2 + TCRT5000×2）。结果在回调里处理。
2. `HAL_UART_Receive_DMA(&huart2, receive_buffer, 1)`：准备接收 1 字节串口/语音指令。
3. `ultrasonic_task1()` / `ultrasonic_task2()`：各触发一次 Trig 脉冲，等待回波中断。
4. OLED 固定标题：桶1 显示 “Other waste”，桶2 显示 “Food waste”。
5. 读取温湿度并显示：`AHT30_Read` / `AHT302_Read` → `OLED_ShowFloat` / `OLED2_ShowFloat`。
6. 报警判定：任一路温度 > 28 °C 则蜂鸣器/红灯报警；否则当烟雾与温度都正常时关闭报警。

> 提示：主循环无阻塞延时，传感器节奏由回调驱动。若要降低功耗或降低刷新率，可加入 `HAL_Delay` 或更精细的状态机。

## 回调/中断逻辑
### 1) 超声波回调 `HAL_TIM_IC_CaptureCallback`
- 作用：根据 Echo 高电平宽度计算距离（cm），距离 ≤ 4 时自动开盖并退出语音模式。
- 行为：
  - 桶1：TIM8 CH3/4；桶2：TIM8 CH1/2。
  - 如果距离大于阈值且当前没有语音保持开盖，则自动关盖。
  - `voice_mode1/2`：舵机由语音/串口打开后，距离回到安全范围才关闭。
- 可调参数：`distance <= 4` 为触发开盖的距离阈值。

### 2) ADC DMA 回调 `HAL_ADC_ConvCpltCallback`
- 作用：处理 4 路模拟值，更新满桶标志与报警。
- 通道对应：`value1` MQ-2(桶1)、`value2` TCRT5000(桶1)、`value3` TCRT5000(桶2)、`value4` MQ-2(桶2)。
- 行为：
  - 桶满判断：`value2/3 < 1000` → `full_flag = 1`，OLED 显示 `Full`；否则 `Not Full`。
  - 烟雾报警：`value1 >= 1000` 或 `value4 >= 1000` → `Beep2_TurnOn()` + `Red_TurnOn()`。
  - 同步刷新两块 OLED 的原始 ADC 值与状态文案。
- 可调参数：`1000` 为桶满与烟雾报警阈值。

### 3) 串口 DMA 回调 `HAL_UART_RxCpltCallback`
- 作用：解析外部命令并执行舵机动作，再立即回显。
- 命令映射：`0x01` 开桶1，`0x02` 关桶1，`0x03` 开桶2，`0x04` 关桶2。
- 防溢出：如果 `full_flag` 已置位，对应 `servo*_open()` 内会忽略开盖。

## 常改参数速查
- 开盖距离：`App/User_Main.c` → `HAL_TIM_IC_CaptureCallback` 中的 `distance <= 4`。
- 烟雾阈值：同文件 `HAL_ADC_ConvCpltCallback` 中 `value1/value4 >= 1000`。
- 满桶阈值：同回调中 `value2/value3 < 1000`。
- 温度报警：主循环中的 `temperature > 28` / `temperature2 > 28`。
- 舵机角度：`Device/servo.c` 中 `__HAL_TIM_SET_COMPARE` 的比较值。

## 典型问题排查
- **舵机不动**：检查 TIM3 PWM 输出是否开启；确认满桶标志是否被置位；5 V 电源是否足够。
- **超声波距离异常**：回波脚是否接到支持复用的定时器通道；目标太近/太远导致测距偏差，可调整阈值。
- **AHT30 无读数**：确认两路 I2C 的上拉、电源；地址冲突时可只保留一路并删除另一套初始化。
- **串口无响应**：波特率与语音模块匹配；DMA 缓冲是否被覆盖；可暂时改用 `HAL_UART_Receive_IT` 做调试。
