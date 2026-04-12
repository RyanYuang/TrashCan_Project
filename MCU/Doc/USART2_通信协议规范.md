# USART2 串口通信协议规范

本文档描述 **TrashCan / EnvCar（气体巡检小车）** 工程中，MCU 与上位机之间经 **USART2** 传输的文本协议。内容以当前仓库源码为准（`uart_protocol`、`protocol_parser`、`GasCar_App` 等）。

---

## 1. 范围与参考实现

| 项目 | 说明 |
|------|------|
| 适用接口 | **USART2**（`PA2` TX，`PA3` RX） |
| 物理层 | UART：8 数据位、无校验、1 停止位（`UART_WORDLENGTH_8B`、`UART_PARITY_NONE`、`UART_STOPBITS_1`） |
| 波特率 | 当前 Cube 配置为 **9600**（见 `Core/Src/usart.c` 中 `MX_USART2_UART_Init`）；若重新生成工程，以实际 `BaudRate` 为准 |
| 成帧 | **行文本协议**：以 **换行符 `LF`（`0x0A`，`\n`）** 标识一帧结束 |
| 最大帧长 | 有效载荷至多 **255 字节**写入行缓冲（另需 `NUL` 结尾），总长受 `UART_BUFFER_SIZE`（256）约束 |
| 接收实现 | 单字节 `HAL_UART_Receive_IT` 累加；**`CR`（`0x0D`，`\r`）不写入缓冲**，可忽略 |

**不在本文档范围（未在 USART2 上启用）**

- `GasCar_App.c` 中 **JC278A 无线模块** 的 `MODE:` / `SPEED:` / `DIST:` / `GAS:` 等示例为 **注释占位**，不是当前 USART2 协议。
- 周期上行 **`$STS:...` 下位机状态帧**（见第 4.3 节），由 `UART_Protocol_SendStatusFrame` 发送，业务在 `EnvCar_App_Task` 中约 **300ms** 调用一次。

---

## 2. 传输层约定

### 2.1 帧格式

```
<有效载荷>\n
```

- 推荐上位机发送：`<有效载荷>\r\n`（`\r` 会被 MCU 丢弃，`\n` 触发帧完成）。
- MCU 侧在收到 `\n` 后，将此前字符视为一帧（不含 `\n`、不含已丢弃的 `\r`），并以 `NUL` 结尾供 `Protocol_Parse` 使用。

### 2.2 字符编码

- 协议载荷为 **ASCII 可打印字符与数字、符号**；数值为十进制浮点/整数文本，与 `protocol_parser` 中 `sscanf` / 手写解析一致。

### 2.3 溢出与未完成帧

- 若一帧长度超过缓冲允许范围，接收状态会重置（丢弃当前帧）。
- 若上一帧已接收完成但应用尚未取走（`UART_RX_COMPLETE`），新到达字节会被丢弃直至 `UART_Protocol_GetFrame` 取走帧并复位状态。

---

## 3. 下行协议（上位机 → MCU）

首字节决定帧类型，由 `Protocol_Parse()` 分发（`protocol_parser.c`）。

### 3.1 控制指令帧：`@<指令码>`

| 字段 | 说明 |
|------|------|
| 前缀 | 单字符 `@`（`0x40`） |
| 指令码 | `@` 之后为十进制整数 **0～10**，前导空格允许；由 `atoi` 解析，**必须为数字开头**（双位数 `10` 合法） |
| 行尾 | `\n`（前可有 `\r`） |

**指令码与枚举 `ControlCommand_t` 对应关系**

| 值 | 符号名 | 含义 |
|----|--------|------|
| 0 | `CMD_STOP` | 停止 |
| 1 | `CMD_FORWARD` | 前进 |
| 2 | `CMD_BACKWARD` | 后退 |
| 3 | `CMD_TURN_LEFT` | 左转 |
| 4 | `CMD_TURN_RIGHT` | 右转 |
| 5 | `CMD_SPEED_25` | 速度 25% |
| 6 | `CMD_SPEED_50` | 速度 50% |
| 7 | `CMD_SPEED_75` | 速度 75% |
| 8 | `CMD_SPEED_100` | 速度 100% |
| 9 | `CMD_MODE_MANUAL` | **手动模式**：`g_EnvCar_Config.mode = ENVCAR_MODE_MANUAL`，由应用层执行（小车等待 `@` 运动/速度指令或其它手动控制） |
| 10 | `CMD_MODE_AUTO_TRACK` | **自动循迹模式**：`g_EnvCar_Config.mode = ENVCAR_MODE_AUTO`（循迹 + 避障 + 气体监测等自动任务） |

**示例**

```text
@0\n          ; 停止
@1\r\n        ; 前进
@5\r\n        ; 25% 速度
@9\r\n        ; 切换为手动模式
@10\r\n       ; 切换为自动循迹模式
```

解析成功后会调用已注册的 `Protocol_Parser_RegisterCommandCallback` 回调；若未注册，仅更新内部“最后指令”状态。

**错误行为**

- 首字符不是 `@`、或 `@` 后非数字、或数值不在 0～10：返回 `PARSE_ERROR`，`g_LastCommand` 置为 `CMD_INVALID`。

---

### 3.2 阈值配置帧：`#O...,G...`（避障 + 气体浓度）

仅配置两类参数：**超声波避障距离门限**（触发距离 + 恢复距离）与 **气体浓度允许区间**（下限、上限）。与旧版温湿度/光照长帧 **不兼容**。

| 字段 | 说明 |
|------|------|
| 前缀 | `#` |
| 固定格式 | `#O` 段 + `G` 段，中间无空格；数值均为十进制 |

**语法**

```text
#O<trig_cm>,<safe_cm>,G<low_ppm>,<high_ppm>\n
```

| 载荷段 | 类型 | 含义 |
|--------|------|------|
| `trig_cm` | 无符号整数 | 障碍物距离 **小于** 该厘米数时，视为过近（触发避障逻辑，对应 `threshold_distance_cm`） |
| `safe_cm` | 无符号整数 | 障碍物距离 **大于** 该厘米数时，视为可恢复安全行驶（对应 `safe_distance_cm`） |
| `low_ppm` / `high_ppm` | 浮点 | 气体浓度允许范围（ppm），对应 `gas_threshold_low_ppm` / `gas_threshold_high_ppm` |

**校验规则（MCU 拒绝时返回 `#ERR:Invalid params`）**

- `0 < trig_cm < safe_cm`，且 `trig_cm`、`safe_cm` 均 **≤ 500**。
- `low_ppm < high_ppm`（严格小于）。
- `low_ppm == 0`：应用层将 **关闭** 浓度下限报警（`enable_low_alarm = false`）；仅使用上限时典型写法为 `#O20,30,G0,500`。

**示例**

```text
#O20,30,G10.0,600.0\r\n
```

**MCU 应答（经 USART2 阻塞发送）**

| 结果 | 上行文本（含行尾） |
|------|-------------------|
| 解析成功并已写入应用配置 | `#OK:Config synced\r\n` |
| 格式不匹配（`sscanf` 未得到 4 个字段） | `#ERR:Parse failed\r\n` |
| 数值未通过校验 | `#ERR:Invalid params\r\n` |

解析成功时先调用 `Protocol_Parser_RegisterThresholdCallback` 注册的回调（工程内将写入 `g_EnvCar_Config`），再发送 `#OK:` 行。

---

### 3.3 其它下行帧

- 首字符既不是 `@` 也不是 `#`：`Protocol_Parse` 返回 **`PARSE_UNKNOWN_TYPE`**。
- 空串或 `NULL`：返回 **`PARSE_ERROR`**。

---

## 4. 上行协议（MCU → 上位机）

### 4.1 回显：`ECHO:`

每收到一帧完整文本（不含行尾的 `\r\n`），应用会在主循环中（`EnvCar_USART2_ProcessHostReply`）发送：

```text
ECHO:<原文>\r\n
```

- `<原文>` 为 MCU 收到的**整行有效载荷**（与进入 `Protocol_Parse` 的字符串一致）。
- 用于调试与确认上位机发送内容；与帧类型（`@` / `#`）无关。

### 4.2 阈值配置应答

见 **第 3.2 节** 表中 `#OK:` / `#ERR:` 两条固定字符串。

### 4.3 下位机状态帧（周期上行）

MCU 经 **`UART_Protocol_SendStatusFrame`** 主动上报（`uart_protocol.c`），建议在 **USART2** 上周期发送（工程中约 **300ms** 一次，带节流）。

**语法**

```text
$STS:<gas_ppm>,<obs_flag>,<obs_cm>,<alarm>,<car_state>,<run_time_s>\r\n
```

| 字段 | 类型 | 说明 |
|------|------|------|
| `gas_ppm` | 浮点文本 | 气体浓度 **ppm**，格式 `%.2f` |
| `obs_flag` | 整数 | **0** = 无障碍（`UART_OBS_NONE`），此时 `obs_cm` 固定为 **0**；**1** = 有障碍（`UART_OBS_NEAR`），表示已小于避障触发距离 |
| `obs_cm` | 整数 | 有障碍时为**最近障碍物距离 (cm)**；无障碍时为 **0** |
| `alarm` | 整数 | **0** 无；**1** 仅障碍条件满足；**2** 仅气体条件满足；**3** 障碍与气体条件**同时**满足（`UART_ALM_NONE` / `_OBST` / `_GAS` / `_BOTH`） |
| `car_state` | 整数 | 与 `EnvCar_State_Enum` 一致：`0` IDLE，`1` TRACKING，`2` OBSTACLE_ALARM，`3` GAS_ALARM，`4` MANUAL_CTRL，`5` ERROR |
| `run_time_s` | 无符号整数 | **系统累计运行时间（秒）**，与 `g_System_Status.system_run_time_s` 一致（`EnvCar_App_Task` 中约每秒 +1） |

**告警字段语义**：`alarm` 由当前**测量值与阈值**组合得到（气体超限或低于下限且使能下限报警；障碍为有效超声距离且小于触发阈值），与 `g_System_Status` 中单一 `alarm_type` 可能不完全同步，但便于上位机同时展示两类风险。

**示例**

```text
$STS:12.50,0,0,0,1,3600\r\n
```

表示气体 12.5 ppm、无障碍、无警报、状态为循迹中、已上电运行 **3600 s**。

```text
$STS:620.00,1,12,3,3,42\r\n
```

表示气体与障碍同时越限、障碍距离 12 cm、警报码 3、状态为气体报警态（具体以运行时 `current_state` 为准）、运行时间 **42 s**。

---

## 5. 软件处理顺序（与当前工程一致）

在 `HAL_UART_RxCpltCallback` 中，当 `huart->Instance == USART2` 时：

1. 调用 `UART_Protocol_RxCallback(huart)` 收字节并判帧结束。
2. 若 `UART_Protocol_IsFrameReady()` 为真：  
   - `UART_Protocol_GetFrame` 取出字符串；  
   - 调用 `EnvCar_USART2_ScheduleHostReply_Isr` 登记待回显内容（**仅拷贝，不在 ISR 内发送**）；  
   - 调用 `Protocol_Parse(rxFrame)` 解析控制/阈值。

主循环中需调用 `EnvCar_USART2_ProcessHostReply()` 才会真正发出 **`ECHO:`** 行。

---

## 6. 设计说明与扩展注意

1. **上行前缀 `$`**  
   状态帧使用 **`$STS:`** 前缀，与下行 **`@` 控制**、**`#` 阈值** 区分，避免与 `@0`～`@10` 指令帧混淆。

2. **调试输出**  
   `Parse_ControlCommand` 中含 `printf("cmdValue:%d\r\n", ...)`，若重定向到同一 UART，可能与协议流混合；量产建议关闭或改用独立调试口。

3. **波特率**  
   文档与示例需与 `usart.c` 中 **USART2 波特率** 保持一致（当前 **9600**）。

---

## 7. 版本与维护

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-04-12 | 初版：依据仓库 `uart_protocol`、`protocol_parser`、`GasCar_App`、`stm32f1xx_it` 整理 |
| 1.1 | 2026-04-12 | 阈值帧由 `#T/H/G/L` 改为 `#O/G`（避障 + 气体）；与 `ProtocolEnvLimits_t` 一致 |
| 1.2 | 2026-04-12 | 上行由传感器四路帧改为 **`$STS:` 下位机状态帧**（气体、障碍标志/距离、组合警报、小车状态） |
| 1.3 | 2026-04-12 | 控制指令扩展 **`@9` 手动模式**、**`@10` 自动循迹模式**；与 `ControlCommand_t`、`EnvCar_OnProtocolCommand` 一致 |
| 1.4 | 2026-04-12 | **`$STS:` 增加第 6 字段 `run_time_s`**（系统运行秒数）；与 `UART_StatusUplink_t` / `EnvCar_StatusUplinkOnce` 一致 |

维护者更新协议时，请同步修改本文件及对应 `.c/.h` 中的注释，避免与 `MCU/Doc/串口协议使用说明.md`（集成教程）描述冲突。

---

## 8. 快速测试清单

| 步骤 | 发送内容（十六进制行尾为 `0D 0A` 或仅 `0A`） | 预期 |
|------|-----------------------------------------------|------|
| 控制 | `@1` + 换行 | 回调收到前进；若有 ECHO 任务则收到 `ECHO:@1` |
| 停止 | `@0` + 换行 | 停止指令 |
| 手动模式 | `@9` + 换行 | 应用层切换为 `ENVCAR_MODE_MANUAL`；ECHO 见 4.1 |
| 自动循迹 | `@10` + 换行 | 应用层切换为 `ENVCAR_MODE_AUTO`；ECHO 见 4.1 |
| 阈值 | 如 `#O20,30,G0,500` 一行 | 成功为 `#OK:Config synced`；格式错为 `#ERR:Parse failed`；校验失败为 `#ERR:Invalid params` |
| 非法 | `ABC` + 换行 | `PARSE_UNKNOWN_TYPE`；仍可能有 `ECHO:ABC` |
| 状态 | （被动接收） | 约每 300ms 收到 `$STS:...` 一行，共 **6 个字段**（逗号分隔，见 4.3 节；末字段为运行秒数） |

---

*文档结束*
