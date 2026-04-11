/**
 * @file Driver_IIC_Platform_CN.h
 * @brief 通用 I2C 软件平台驱动头文件（中文注释版）
 * @details 这是一个跨平台的 I2C 位操作（bit-banging）驱动，支持多种 MCU 平台。
 *          用户需要在 iic_platform_config.h 中配置平台相关的宏定义
 * @version 1.0
 * @date 2025-11-22
 * @author RyanYuang
 */

#ifndef _DRIVER_IIC_PLATFORM_H
#define _DRIVER_IIC_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "Driver_IIC/IIC_Platform/CN/iic_platform_config_CN.h"

/*============================================================================*/
/*                         读写操作标志                                        */
/*============================================================================*/

/**
 * @defgroup IIC_Operation I2C 操作标志
 * @brief I2C 通信的读写操作标志
 * @{
 */
#define WRITE   0  /**< 写操作 */
#define READ    1  /**< 读操作 */
/** @} */

/*============================================================================*/
/*                         应答信号定义                                        */
/*============================================================================*/

/**
 * @defgroup IIC_ACK I2C 应答标志
 * @brief I2C 通信的应答/非应答标志
 * @{
 */
#define NO_ACK  0  /**< 非应答（NACK） */
#define ACK     1  /**< 应答（ACK） */
/** @} */

/*============================================================================*/
/*                         设备 ID 定义                                        */
/*============================================================================*/

/**
 * @defgroup IIC_Device_ID I2C 设备 ID
 * @brief 常用 I2C 设备的预定义 ID
 * @note 用户可根据需要添加更多设备 ID
 * @{
 */
#define SSD1306     0  /**< SSD1306 OLED 显示屏 */
#define INA226      1  /**< INA226 功率监测芯片 */
#define M24C64      2  /**< M24C64 EEPROM 存储器 */
#define MPU6050     3  /**< MPU6050 六轴传感器 */
#define IMU963RA    4  /**< IMU963RA 惯性测量单元 */
/** @} */

/*============================================================================*/
/*                         错误码定义                                          */
/*============================================================================*/

/**
 * @enum IIC_Error_Code
 * @brief I2C 错误码
 * @details I2C 函数返回这些错误码来指示操作状态
 */
enum IIC_Error_Code {
    IIC_OK              = 0,   /**< 操作成功 */
    IIC_Error_ACK       = -1,  /**< 通用应答错误 */
    IIC_Error_ACK_REG   = -2,  /**< 寄存器地址应答错误 */
    IIC_Error_ACK_DATA  = -3,  /**< 数据字节应答错误 */
    IIC_Error_ACK_ADDR  = -4,  /**< 设备地址应答错误 */
};

/*============================================================================*/
/*                         结构体定义                                          */
/*============================================================================*/

/**
 * @struct I2C_Platform_Structure
 * @brief I2C 平台结构体
 * @details 包含一个 I2C 实例的 SDA 和 SCL 引脚信息
 */
typedef struct I2C_Platform_Structure {
    Pin_Struct SDA;  /**< SDA（数据）引脚配置 */
    Pin_Struct SCL;  /**< SCL（时钟）引脚配置 */
} I2C_Platform_Structure;

/*============================================================================*/
/*                         驱动配置                                            */
/*============================================================================*/

/**
 * @def Platform_Maximum_Driver
 * @brief I2C 驱动实例的最大数量
 * @details 定义可以注册的 I2C 实例的最大数量
 * @note 根据系统需求调整此值
 */
#define Platform_Maximum_Driver 10

/**
 * @var I2C_Driver
 * @brief 全局 I2C 驱动实例数组
 * @details 存储所有已注册的 I2C 驱动实例
 */
extern I2C_Platform_Structure I2C_Driver[Platform_Maximum_Driver];

/*============================================================================*/
/*                         底层 I2C 驱动函数                                   */
/*============================================================================*/

/**
 * @defgroup IIC_Driver_API I2C 底层驱动 API
 * @brief 底层 I2C 位操作驱动函数
 * @{
 */

/**
 * @brief 注册一个 I2C 驱动实例
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] SDA 指向 SDA 引脚结构体的指针
 * @param[in] SCL 指向 SCL 引脚结构体的指针
 * @return 无
 * @note 在使用其他 I2C 函数之前必须先调用此函数
 * @par 示例:
 * @code
 * Pin_Struct sda_pin = {GPIOB, GPIO_PIN_7};
 * Pin_Struct scl_pin = {GPIOB, GPIO_PIN_6};
 * IIC_Register(&I2C_Driver[0], &sda_pin, &scl_pin);
 * @endcode
 */
void IIC_Register(I2C_Platform_Structure* i2c, Pin_Struct *SDA, Pin_Struct *SCL);

/**
 * @brief 产生 I2C 起始信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 * @details 产生 I2C 起始条件：SCL 为高时，SDA 由高变低
 * @note 此函数应在 I2C 传输开始时调用
 */
void IIC_Start(I2C_Platform_Structure* i2c);

/**
 * @brief 产生 I2C 停止信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 * @details 产生 I2C 停止条件：SCL 为高时，SDA 由低变高
 * @note 此函数应在 I2C 传输结束时调用
 */
void IIC_Stop(I2C_Platform_Structure* i2c);

/**
 * @brief 等待 I2C 应答信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 应答状态
 * @retval 0 收到应答（ACK，成功）
 * @retval 1 收到非应答（NACK，失败）
 * @details 等待从设备将 SDA 拉低（应答）
 */
uint8_t IIC_Wait_Ack(I2C_Platform_Structure* i2c);

/**
 * @brief 通过 I2C 发送一个字节
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Byte 要发送的数据字节
 * @return 无
 * @details 发送 8 位数据，高位在前（MSB first）
 */
void IIC_Send_Byte(I2C_Platform_Structure* i2c, uint8_t Byte);

/**
 * @brief 通过 I2C 读取一个字节
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] ack 读取后发送的应答/非应答标志
 *                - ACK (1): 读取后发送应答（继续读取）
 *                - NO_ACK (0): 读取后发送非应答（停止读取）
 * @return 接收到的数据字节
 * @details 读取 8 位数据，高位在前（MSB first）
 */
uint8_t IIC_Read_Byte(I2C_Platform_Structure* i2c, uint8_t ack);

/**
 * @brief 扫描 I2C 总线上的设备
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 找到的设备数量
 * @details 扫描所有可能的 I2C 地址（0x00-0x7F）并打印找到的设备
 * @note 此函数使用 printf 输出，请确保 IIC_DEBUG_ENABLE 已设置
 */
uint8_t IIC_Scan(I2C_Platform_Structure* i2c);

/** @} */

/*============================================================================*/
/*                         高层寄存器操作                                      */
/*============================================================================*/

/**
 * @defgroup IIC_Register_API I2C 寄存器操作 API
 * @brief 高层 I2C 寄存器读写函数
 * @{
 */

/**
 * @brief 从 I2C 设备寄存器读取数据
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Device_address I2C 设备地址（7位，不含读写位）
 * @param[in] Reg_address 要读取的寄存器地址
 * @param[in] Size 要读取的字节数
 * @param[out] Data_buf 存储读取数据的缓冲区
 * @return 来自 enum IIC_Error_Code 的错误码
 * @retval IIC_OK 操作成功
 * @retval IIC_Error_ACK_ADDR 设备地址应答错误
 * @retval IIC_Error_ACK_REG 寄存器地址应答错误
 * @par 示例:
 * @code
 * uint8_t data[2];
 * uint8_t result = IIC_Read_Reg(&I2C_Driver[0], 0x68, 0x75, 2, data);
 * if (result == IIC_OK) {
 *     // 数据读取成功
 * }
 * @endcode
 */
uint8_t IIC_Read_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,
    uint8_t Reg_address,
    uint8_t Size,
    uint8_t Data_buf[]
);

/**
 * @brief 向 I2C 设备寄存器写入数据
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Device_address I2C 设备地址（7位，不含读写位）
 * @param[in] Reg_address 要写入的寄存器地址
 * @param[in] Size 要写入的字节数
 * @param[in] Data_buf 指向数据缓冲区的指针
 * @return 来自 enum IIC_Error_Code 的错误码
 * @retval IIC_OK 操作成功
 * @retval IIC_Error_ACK_ADDR 设备地址应答错误
 * @retval IIC_Error_ACK_REG 寄存器地址应答错误
 * @retval IIC_Error_ACK_DATA 数据字节应答错误
 * @par 示例:
 * @code
 * uint8_t data[] = {0x01, 0x02};
 * uint8_t result = IIC_Write_Reg(&I2C_Driver[0], 0x68, 0x6B, 2, data);
 * if (result == IIC_OK) {
 *     // 数据写入成功
 * }
 * @endcode
 */
uint8_t IIC_Write_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,
    uint8_t Reg_address,
    uint8_t Size,
    const void* Data_buf
);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _DRIVER_IIC_PLATFORM_H */
