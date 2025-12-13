/**
 * @file iic_platform_config_CN.h
 * @brief IIC 平台配置文件 - 用户需要实现（中文版）
 * @details 此文件定义了平台相关的宏和接口，用户需要根据不同的 MCU 平台
 *          （STM32、ESP32、AVR 等）来实现这些宏
 * @version 1.0
 * @date 2025-11-22
 * @author RyanYuang
 * @note 用户必须根据自己的平台实现此文件中定义的所有宏
 */

#ifndef _IIC_PLATFORM_CONFIG_H
#define _IIC_PLATFORM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/* 在此包含您的平台相关头文件 */
/* STM32 示例: #include "stm32xxxx_hal.h" */
/* ESP32 示例: #include "driver/gpio.h" */
/* AVR 示例: #include <avr/io.h> */

/* 在此添加您的平台头文件 */
/* HAL */
#ifdef USE_HAL_DRIVER

#include "main.h"
#include "Drivers.h"

#endif

/*============================================================================*/
/*                         引脚结构体定义                                      */
/*============================================================================*/

/**
 * @brief 引脚结构体定义
 * @details 用户应该根据自己的平台定义此结构体
 * @note STM32 HAL库: typedef struct { GPIO_TypeDef* Port; uint16_t Pin; } Pin_Struct;
 * @note STM32 标准库: typedef struct { GPIO_TypeDef* Port; uint16_t Pin; } Pin_Struct;
 * @note ESP32: typedef struct { gpio_num_t Pin; } Pin_Struct;
 * @note AVR: typedef struct { volatile uint8_t* Port; uint8_t Pin; } Pin_Struct;
 */
/* Pin_Struct 应该在您的平台头文件中定义 */


/*============================================================================*/
/*                         延时配置                                           */
/*============================================================================*/

/**
 * @brief IIC 延时时间（微秒）
 * @details 根据您的 I2C 时钟速度需求调整此值
 *          典型值：400kHz 使用 1us，100kHz 使用 5us
 */
#define IIC_DELAY_US 1

/**
 * @brief 阻塞延时函数
 * @details 此宏应调用一个微秒级延时函数（阻塞式）
 * @param us 延时时间（微秒）
 * @note STM32 HAL库: #define IIC_DELAY_BLOCKING(us) HAL_Delay_us(us)  // 需自行实现微秒延时
 * @note STM32 标准库: #define IIC_DELAY_BLOCKING(us) delay_us(us)      // 或使用 SysTick 实现
 * @note STM32 通用: #define IIC_DELAY_BLOCKING(us) SysTick_Delay_us(us)
 * @note ESP32: #define IIC_DELAY_BLOCKING(us) esp_rom_delay_us(us)
 * @note AVR: #define IIC_DELAY_BLOCKING(us) _delay_us(us)
 */
#define IIC_DELAY_BLOCKING(us) SysTick_Delay_us(us)

/**
 * @brief 非阻塞延时函数（可选）
 * @details 如果您想支持非阻塞操作，请实现此宏
 *          否则，可以将其定义为 IIC_DELAY_BLOCKING
 * @param us 延时时间（微秒）
 * @note 非阻塞实现可以使用系统定时器或 RTOS 延时函数
 */
#define IIC_DELAY_NONBLOCKING(us) IIC_DELAY_BLOCKING(us)

/*============================================================================*/
/*                         GPIO 控制宏                                        */
/*============================================================================*/

/**
 * @brief 将 GPIO 引脚设置为高电平
 * @param pin 指向 Pin_Struct 的指针
 * @note STM32 HAL库: HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_SET)
 * @note STM32 标准库: GPIO_SetBits((pin)->Port, (pin)->Pin)
 * @note ESP32: gpio_set_level((pin)->Pin, 1)
 * @note AVR: *((pin)->Port) |= (1 << (pin)->Pin)
 */
#define IIC_GPIO_SET(pin) HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_SET)

/**
 * @brief 将 GPIO 引脚设置为低电平
 * @param pin 指向 Pin_Struct 的指针
 * @note STM32 HAL库: HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_RESET)
 * @note STM32 标准库: GPIO_ResetBits((pin)->Port, (pin)->Pin)
 * @note ESP32: gpio_set_level((pin)->Pin, 0)
 * @note AVR: *((pin)->Port) &= ~(1 << (pin)->Pin)
 */
#define IIC_GPIO_RESET(pin) HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_RESET)

/**
 * @brief 读取 GPIO 引脚电平
 * @param pin 指向 Pin_Struct 的指针
 * @return 引脚电平（0 或 1）
 * @note STM32 HAL库: HAL_GPIO_ReadPin((pin)->Port, (pin)->Pin)
 * @note STM32 标准库: GPIO_ReadInputDataBit((pin)->Port, (pin)->Pin)
 * @note ESP32: gpio_get_level((pin)->Pin)
 * @note AVR: (*((pin)->Port) & (1 << (pin)->Pin))
 */
#define IIC_GPIO_READ(pin) HAL_GPIO_ReadPin((pin)->Port, (pin)->Pin)

/*============================================================================*/
/*                         工作模式配置                                        */
/*============================================================================*/

/**
 * @brief IIC 工作模式选择
 * @details 设置为 1 使用阻塞模式，设置为 0 使用非阻塞模式
 * @note 阻塞模式：简单可靠，CPU 在延时期间等待
 * @note 非阻塞模式：更高效，需要正确实现定时器/RTOS 延时
 */
#define IIC_BLOCKING_MODE 1

#if IIC_BLOCKING_MODE
    #define IIC_DELAY() IIC_DELAY_BLOCKING(IIC_DELAY_US)
#else
    #define IIC_DELAY() IIC_DELAY_NONBLOCKING(IIC_DELAY_US)
#endif

/*============================================================================*/
/*                         调试输出配置（可选）                                */
/*============================================================================*/

/**
 * @brief 启用/禁用调试输出
 * @details 设置为 1 启用 printf 调试输出，设置为 0 禁用
 */
#define IIC_DEBUG_ENABLE 1

#if IIC_DEBUG_ENABLE
    #include <stdio.h>
    /**
     * @brief 调试打印宏
     * @param fmt 格式化字符串（与 printf 相同）
     * @note 示例: IIC_DEBUG_PRINT("设备地址: 0x%X\r\n", addr);
     */
    #define IIC_DEBUG_PRINT(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define IIC_DEBUG_PRINT(fmt, ...)
#endif

/*============================================================================*/
/*                         平台配置说明                                        */
/*============================================================================*/

/**
 * @section platform_examples 平台配置示例
 *
 * @subsection stm32_hal_example STM32 HAL库 平台示例
 * @code
 * #include "stm32f1xx_hal.h"  // 根据具体芯片型号修改
 * // Pin_Struct 在您的 HeadFiles.h 中定义
 * // typedef struct { GPIO_TypeDef* Port; uint16_t Pin; } Pin_Struct;
 *
 * #define IIC_DELAY_BLOCKING(us) SysTick_Delay_us(us)  // 需自行实现微秒延时函数
 * #define IIC_GPIO_SET(pin) HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_SET)
 * #define IIC_GPIO_RESET(pin) HAL_GPIO_WritePin((pin)->Port, (pin)->Pin, GPIO_PIN_RESET)
 * #define IIC_GPIO_READ(pin) HAL_GPIO_ReadPin((pin)->Port, (pin)->Pin)
 * @endcode
 *
 * @subsection stm32_std_example STM32 标准库 平台示例
 * @code
 * #include "stm32f10x.h"  // 根据具体芯片型号修改
 * #include "stm32f10x_gpio.h"
 * // Pin_Struct 在您的 HeadFiles.h 中定义
 * // typedef struct { GPIO_TypeDef* Port; uint16_t Pin; } Pin_Struct;
 *
 * #define IIC_DELAY_BLOCKING(us) delay_us(us)  // 使用标准库延时函数
 * #define IIC_GPIO_SET(pin) GPIO_SetBits((pin)->Port, (pin)->Pin)
 * #define IIC_GPIO_RESET(pin) GPIO_ResetBits((pin)->Port, (pin)->Pin)
 * #define IIC_GPIO_READ(pin) GPIO_ReadInputDataBit((pin)->Port, (pin)->Pin)
 * @endcode
 *
 * @subsection esp32_example ESP32 平台示例
 * @code
 * #include "driver/gpio.h"
 * #include "rom/ets_sys.h"
 * typedef struct { gpio_num_t Pin; } Pin_Struct;
 * #define IIC_DELAY_BLOCKING(us) esp_rom_delay_us(us)
 * #define IIC_GPIO_SET(pin) gpio_set_level((pin)->Pin, 1)
 * #define IIC_GPIO_RESET(pin) gpio_set_level((pin)->Pin, 0)
 * #define IIC_GPIO_READ(pin) gpio_get_level((pin)->Pin)
 * @endcode
 *
 * @subsection avr_example AVR 平台示例
 * @code
 * #include <avr/io.h>
 * #include <util/delay.h>
 * typedef struct { volatile uint8_t* Port; uint8_t Pin; } Pin_Struct;
 * #define IIC_DELAY_BLOCKING(us) _delay_us(us)
 * #define IIC_GPIO_SET(pin) (*((pin)->Port) |= (1 << (pin)->Pin))
 * #define IIC_GPIO_RESET(pin) (*((pin)->Port) &= ~(1 << (pin)->Pin))
 * #define IIC_GPIO_READ(pin) ((*((pin)->Port) & (1 << (pin)->Pin)) ? 1 : 0)
 * @endcode
 */

/*============================================================================*/
/*                         配置步骤说明                                        */
/*============================================================================*/

/**
 * @section config_steps 配置步骤
 *
 * @par 步骤 1: 包含平台头文件
 * 在文件开头包含您的平台相关头文件，例如：
 * - STM32: #include "stm32f1xx_hal.h"
 * - ESP32: #include "driver/gpio.h"
 * - AVR: #include <avr/io.h>
 *
 * @par 步骤 2: 定义引脚结构体（如果需要）
 * 确保 Pin_Struct 结构体已在您的头文件中定义，包含平台所需的引脚信息
 *
 * @par 步骤 3: 实现延时宏
 * 根据您的平台实现 IIC_DELAY_BLOCKING(us) 宏，确保能够实现微秒级精确延时
 *
 * @par 步骤 4: 实现 GPIO 控制宏
 * 实现 IIC_GPIO_SET、IIC_GPIO_RESET、IIC_GPIO_READ 三个宏
 *
 * @par 步骤 5: 配置工作模式
 * 选择阻塞或非阻塞模式（建议初学者使用阻塞模式）
 *
 * @par 步骤 6: 配置调试输出（可选）
 * 根据需要启用或禁用调试输出
 */

/**
 * @section gpio_requirements GPIO 配置要求
 *
 * @par I2C 引脚要求：
 * - 模式：开漏输出（Open-Drain）或推挽输出配合外部上拉电阻
 * - 上拉：必须有上拉电阻（内部或外部，推荐 2.2K-10K）
 * - 速度：高速或超高速
 * - 初始状态：高电平
 *
 * @par STM32 HAL库 GPIO 配置示例：
 * @code
 * void I2C_GPIO_Init_HAL(void)
 * {
 *     GPIO_InitTypeDef GPIO_InitStruct = {0};
 *
 *     // 使能 GPIO 时钟
 *     __HAL_RCC_GPIOB_CLK_ENABLE();
 *
 *     // 配置 GPIO 引脚：PB6 (SCL) 和 PB7 (SDA)
 *     GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
 *     GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;  // 开漏输出
 *     GPIO_InitStruct.Pull = GPIO_PULLUP;           // 内部上拉
 *     GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
 *     HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
 *
 *     // 设置初始状态为高电平
 *     HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
 * }
 * @endcode
 *
 * @par STM32 标准库 GPIO 配置示例：
 * @code
 * void I2C_GPIO_Init_STD(void)
 * {
 *     GPIO_InitTypeDef GPIO_InitStruct;
 *
 *     // 使能 GPIO 时钟
 *     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
 *
 *     // 配置 GPIO 引脚：PB6 (SCL) 和 PB7 (SDA)
 *     GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
 *     GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;  // 开漏输出
 *     GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
 *     GPIO_Init(GPIOB, &GPIO_InitStruct);
 *
 *     // 设置初始状态为高电平
 *     GPIO_SetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
 * }
 * @endcode
 *
 * @note HAL库和标准库的主要区别：
 * - HAL库：使用 GPIO_MODE_OUTPUT_OD，内部上拉通过 GPIO_PULLUP 配置
 * - 标准库：使用 GPIO_Mode_Out_OD，通常需要外部上拉电阻（内部上拉较弱）
 * - HAL库的结构体成员名以大写开头，标准库以 GPIO_ 前缀开头
 */

/**
 * @section usage_example 使用示例
 *
 * @code
 * // 1. 包含驱动头文件
 * #include "IIC_Platform.h"
 *
 * // 2. 定义 GPIO 引脚（确保已配置为开漏或推挽+上拉）
 * Pin_Struct i2c_sda = {GPIOB, GPIO_PIN_7};  // SDA 在 PB7
 * Pin_Struct i2c_scl = {GPIOB, GPIO_PIN_6};  // SCL 在 PB6
 *
 * // 3. 初始化 I2C 平台驱动
 * void I2C_Init(void)
 * {
 *     // 注册 I2C 驱动实例 0
 *     IIC_Register(&I2C_Driver[0], &i2c_sda, &i2c_scl);
 * }
 *
 * // 4. 扫描 I2C 设备
 * void I2C_ScanDevices(void)
 * {
 *     uint8_t device_count = IIC_Scan(&I2C_Driver[0]);
 *     printf("找到 %d 个 I2C 设备\r\n", device_count);
 * }
 *
 * // 5. 从 I2C 设备寄存器读取数据
 * void I2C_ReadExample(void)
 * {
 *     uint8_t data[2];
 *     uint8_t result = IIC_Read_Reg(&I2C_Driver[0], 0x68, 0x75, 2, data);
 *
 *     if (result == IIC_OK) {
 *         printf("读取成功: 0x%02X 0x%02X\r\n", data[0], data[1]);
 *     } else {
 *         printf("读取失败，错误码: %d\r\n", result);
 *     }
 * }
 *
 * // 6. 向 I2C 设备寄存器写入数据
 * void I2C_WriteExample(void)
 * {
 *     uint8_t data[] = {0x01, 0x02};
 *     uint8_t result = IIC_Write_Reg(&I2C_Driver[0], 0x68, 0x6B, 2, data);
 *
 *     if (result == IIC_OK) {
 *         printf("写入成功\r\n");
 *     } else {
 *         printf("写入失败，错误码: %d\r\n", result);
 *     }
 * }
 * @endcode
 */

#ifdef __cplusplus
}
#endif

#endif /* _IIC_PLATFORM_CONFIG_H */
