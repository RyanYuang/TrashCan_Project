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
- `UART_Protocol_SendSensorData()` 提供的 **`@温度,湿度,CO,光照\r\n`** 上行格式为库函数能力；**当前业务代码未检索到调用**。若将来与下行 `@指令码` 共用同一路径，需注意语义区分（见第 6 节）。

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
| 指令码 | `@` 之后为十进制整数 **0～8**，前导空格允许；由 `atoi` 解析，**必须为数字开头** |
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

**示例**

```text
@0\n          ; 停止
@1\r\n        ; 前进
@5\r\n        ; 25% 速度
```

解析成功后会调用已注册的 `Protocol_Parser_RegisterCommandCallback` 回调；若未注册，仅更新内部“最后指令”状态。

**错误行为**

- 首字符不是 `@`、或 `@` 后非数字、或数值不在 0～8：返回 `PARSE_ERROR`，`g_LastCommand` 置为 `CMD_INVALID`。

---

### 3.2 阈值配置帧：`#T...H...G...L...`

| 字段 | 说明 |
|------|------|
| 前缀 | `#` |
| 结构 | 固定分段顺序：`T` 温度 → `H` 湿度 → `G` CO → `L` 光照；每段为两个浮点数，逗号分隔 |

**语法（逻辑格式）**

```text
#T<temp_low>,<temp_high>,H<hum_low>,<hum_high>,G<co_warning>,<co_danger>,L<light_low>,<light_high>\n
```

**`ThresholdConfig_t` 字段含义**（与 `threshold_config.h` 一致）

| 字段 | 单位 | 说明 |
|------|------|------|
| `temp_low` / `temp_high` | °C | 温度下限 / 上限 |
| `hum_low` / `hum_high` | % | 湿度下限 / 上限 |
| `co_warning` / `co_danger` | ppm | CO 警告 / 危险阈值 |
| `light_low` / `light_high` | lux | 光照下限 / 上限 |

**示例**

```text
#T-10.0,45.0,H20.0,90.0,G30.0,50.0,L0.0,10000.0\r\n
```

**MCU 应答（经 USART2 阻塞发送）**

| 结果 | 上行文本（含行尾） |
|------|-------------------|
| 解析成功 | `#OK:Config synced\r\n` |
| 解析失败 | `#ERR:Parse failed\r\n` |

解析成功时还会调用 `Protocol_Parser_RegisterThresholdCallback` 注册的回调，将新配置下发给应用层。

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

### 4.3 传感器数据帧（库函数，可选）

函数 `UART_Protocol_SendSensorData()`（`uart_protocol.c`）定义为：

```text
@<温度>,<湿度>,<CO>,<光照>\r\n
```

- 温度、湿度、CO：格式 `%.1f`（一位小数）；光照：`%.0f`（整数）。
- **当前工程未在业务路径中调用该函数**；若启用，上位机需与 `@0`～`@8` 控制帧区分（见第 6 节）。

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

1. **`@` 语义重叠**  
   - 控制帧：`@` + **单一指令码 0～8**（其后非数字即结束数值部分）。  
   - 传感器上行（若使用 `UART_Protocol_SendSensorData`）：`@` + **多个逗号分隔浮点**。  
   若同端口混用，上位机解析侧应依据长度与逗号区分，或改为不同前缀（扩展时建议修改协议版本或前缀）。

2. **调试输出**  
   `Parse_ControlCommand` 中含 `printf("cmdValue:%d\r\n", ...)`，若重定向到同一 UART，可能与协议流混合；量产建议关闭或改用独立调试口。

3. **波特率**  
   文档与示例需与 `usart.c` 中 **USART2 波特率** 保持一致（当前 **9600**）。

---

## 7. 版本与维护

| 版本 | 日期 | 说明 |
|------|------|------|
| 1.0 | 2026-04-12 | 初版：依据仓库 `uart_protocol`、`protocol_parser`、`GasCar_App`、`stm32f1xx_it` 整理 |

维护者更新协议时，请同步修改本文件及对应 `.c/.h` 中的注释，避免与 `MCU/Doc/串口协议使用说明.md`（集成教程）描述冲突。

---

## 8. 快速测试清单

| 步骤 | 发送内容（十六进制行尾为 `0D 0A` 或仅 `0A`） | 预期 |
|------|-----------------------------------------------|------|
| 控制 | `@1` + 换行 | 回调收到前进；若有 ECHO 任务则收到 `ECHO:@1` |
| 停止 | `@0` + 换行 | 停止指令 |
| 阈值 | 见 3.2 节示例一行 | 收到 `#OK:Config synced` 或 `#ERR:Parse failed` |
| 非法 | `ABC` + 换行 | `PARSE_UNKNOWN_TYPE`；仍可能有 `ECHO:ABC` |

---

*文档结束*
