# Universal I2C Platform Driver

[中文版](#中文说明) | [English](#english-version)

---

## 中文说明

### 概述

这是一个跨平台的 I2C 软件驱动库，支持多种 MCU 平台（STM32、ESP32、AVR 等）。采用位操作（bit-banging）方式实现 I2C 通信，无需硬件 I2C 外设。

### 主要特性

- ✅ **跨平台支持**：通过宏配置支持多种 MCU 平台
- ✅ **灵活配置**：用户可自定义延时函数、GPIO 操作
- ✅ **阻塞/非阻塞模式**：支持两种工作模式
- ✅ **完整的 Doxygen 注释**：详细的 API 文档
- ✅ **多设备支持**：可注册最多 10 个 I2C 实例
- ✅ **错误处理**：完善的错误码机制
- ✅ **设备扫描**：内置 I2C 总线扫描功能

### 文件结构

```
IIC_Platform/
├── Driver_IIC_Platform.h         # 主头文件（完整的 Doxygen 注释）
├── Driver_IIC_Platform.c         # 实现文件
├── iic_platform_config.h         # 平台配置文件（用户需实现）
├── examples/                     # 平台配置示例
│   ├── stm32_config_example.h   # STM32 平台示例
│   ├── esp32_config_example.h   # ESP32 平台示例
│   └── avr_config_example.h     # AVR 平台示例
└── README.md                     # 本文件
```

### 快速开始

#### 1. 配置平台

编辑 `iic_platform_config.h`，根据你的平台实现以下宏：

```c
/* 必须实现的宏 */
#define IIC_DELAY_BLOCKING(us)     // 阻塞延时函数
#define IIC_GPIO_SET(pin)          // GPIO 置高
#define IIC_GPIO_RESET(pin)        // GPIO 置低
#define IIC_GPIO_READ(pin)         // 读取 GPIO 电平

/* 可选配置 */
#define IIC_DELAY_US 1             // 延时时间（微秒）
#define IIC_BLOCKING_MODE 1        // 1=阻塞模式，0=非阻塞模式
#define IIC_DEBUG_ENABLE 1         // 是否启用调试输出
```

#### 2. STM32 平台示例

```c
#include "Driver_IIC_Platform.h"

// 定义 I2C 引脚
Pin_Struct i2c_sda = {GPIOB, GPIO_PIN_7};  // SDA: PB7
Pin_Struct i2c_scl = {GPIOB, GPIO_PIN_6};  // SCL: PB6

int main(void)
{
    // 初始化 HAL 库
    HAL_Init();
    SystemClock_Config();

    // 注册 I2C 驱动实例 0
    IIC_Driver_register(0, &i2c_sda, &i2c_scl);

    // 扫描 I2C 总线
    I2C_Scan(&I2C_Driver[0]);

    // 读取寄存器
    uint8_t data[2];
    if (IIC_Read_Reg(&I2C_Driver[0], 0x68, 0x75, 2, data) == IIC_OK) {
        printf("读取成功: 0x%02X 0x%02X\r\n", data[0], data[1]);
    }

    // 写入寄存器
    uint8_t write_data[] = {0x01, 0x02};
    IIC_Write_Reg(&I2C_Driver[0], 0x68, 0x6B, 2, write_data);

    while(1) {
        // 应用代码
    }
}
```

#### 3. GPIO 配置注意事项

I2C 引脚应配置为：
- **模式**：开漏输出（Open-Drain）或推挽输出+外部上拉
- **上拉**：启用内部上拉或外部上拉电阻（2.2K-10K）
- **速度**：高速或超高速

**STM32 GPIO 配置示例：**

```c
void I2C_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;    // 开漏输出
    GPIO_InitStruct.Pull = GPIO_PULLUP;            // 内部上拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 初始状态为高电平
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
}
```

### API 参考

#### 驱动管理函数

```c
// 注册 I2C 驱动实例
void IIC_Driver_register(uint8_t ordinal, Pin_Struct *SDA, Pin_Struct *SCL);

// 扫描 I2C 总线设备
uint8_t I2C_Scan(I2C_Platform_Structure* i2c);
```

#### 底层驱动函数

```c
// I2C 起始信号
void Driver_IIC_Start(I2C_Platform_Structure* i2c);

// I2C 停止信号
void Driver_IIC_Stop(I2C_Platform_Structure* i2c);

// 等待 ACK 应答
uint8_t Driver_IIC_Wait_Ack(I2C_Platform_Structure* i2c);

// 发送一个字节
void Driver_IIC_Send_Byte(I2C_Platform_Structure* i2c, uint8_t Byte);

// 读取一个字节
uint8_t Driver_IIC_Read_Byte(I2C_Platform_Structure* i2c, uint8_t ack);
```

#### 寄存器操作函数

```c
// 读取寄存器
uint8_t IIC_Read_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,   // 7位设备地址
    uint8_t Reg_address,      // 寄存器地址
    uint8_t Size,             // 读取字节数
    uint8_t Data_buf[]        // 数据缓冲区
);

// 写入寄存器
uint8_t IIC_Write_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,   // 7位设备地址
    uint8_t Reg_address,      // 寄存器地址
    uint8_t Size,             // 写入字节数
    const void* Data_buf      // 数据缓冲区
);
```

### 错误码

```c
enum IIC_Error_Code {
    IIC_OK              = 0,   // 操作成功
    IIC_Error_ACK       = -1,  // 通用 ACK 错误
    IIC_Error_ACK_REG   = -2,  // 寄存器地址 ACK 错误
    IIC_Error_ACK_DATA  = -3,  // 数据字节 ACK 错误
    IIC_Error_ACK_ADDR  = -4,  // 设备地址 ACK 错误
};
```

### 支持的平台示例

| 平台 | 配置文件 | Pin_Struct 定义 |
|------|---------|----------------|
| STM32 | `examples/stm32_config_example.h` | `{GPIO_TypeDef* Port, uint16_t Pin}` |
| ESP32 | `examples/esp32_config_example.h` | `{gpio_num_t Pin}` |
| AVR | `examples/avr_config_example.h` | `{volatile uint8_t* Port, uint8_t Pin}` |

### 常见问题

**Q: I2C 通信失败怎么办？**
- 检查 GPIO 是否配置为开漏模式
- 确认是否有上拉电阻（内部或外部）
- 验证 I2C 设备地址是否正确
- 使用 `I2C_Scan()` 扫描总线上的设备

**Q: 如何调整 I2C 速度？**
- 修改 `IIC_DELAY_US` 的值
- 400kHz: `IIC_DELAY_US = 1`
- 100kHz: `IIC_DELAY_US = 5`

**Q: 是否支持硬件 I2C？**
- 本驱动是纯软件实现，不依赖硬件 I2C 外设
- 可以在任意 GPIO 引脚上使用

---

## English Version

### Overview

A cross-platform I2C software driver library supporting multiple MCU platforms (STM32, ESP32, AVR, etc.). Implements I2C communication using bit-banging, requiring no hardware I2C peripheral.

### Features

- ✅ **Cross-platform**: Supports multiple MCU platforms via macro configuration
- ✅ **Flexible configuration**: User-defined delay and GPIO operations
- ✅ **Blocking/Non-blocking modes**: Two operating modes supported
- ✅ **Complete Doxygen documentation**: Detailed API documentation
- ✅ **Multi-device support**: Up to 10 I2C instances
- ✅ **Error handling**: Comprehensive error code mechanism
- ✅ **Device scanning**: Built-in I2C bus scanning

### Quick Start

See examples in `examples/` directory for platform-specific configurations:
- `stm32_config_example.h` - STM32 platform
- `esp32_config_example.h` - ESP32 platform
- `avr_config_example.h` - AVR platform

### License

This is a universal platform driver. Users are free to modify and use according to their needs.

### Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.

---

**版本**: 1.0
**日期**: 2025-11-22
**作者**: RyanYuang
