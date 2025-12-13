/**
 * @file IIC_Platform_CN.c
 * @brief 通用 I2C 软件平台驱动实现（中文注释版）
 * @details 此文件实现了跨平台的 I2C 位操作驱动，
 *          通过适当的配置可在任何 MCU 平台上工作
 * @version 1.0
 * @date 2025-11-22
 * @author RyanYuang
 */

#include "IIC_Platform.h"

/*============================================================================*/
/*                         全局变量                                           */
/*============================================================================*/

/**
 * @brief 全局 I2C 驱动实例数组
 */
I2C_Platform_Structure I2C_Driver[Platform_Maximum_Driver];

/*============================================================================*/
/*                         内部辅助函数                                        */
/*============================================================================*/

/**
 * @brief 将 SCL 引脚设置为高电平
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 */
static inline void SCL_SET(I2C_Platform_Structure* i2c)
{
    IIC_GPIO_SET(&(i2c->SCL));
}

/**
 * @brief 将 SCL 引脚设置为低电平
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 */
static inline void SCL_RESET(I2C_Platform_Structure* i2c)
{
    IIC_GPIO_RESET(&(i2c->SCL));
}

/**
 * @brief 将 SDA 引脚设置为高电平
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 */
static inline void SDA_SET(I2C_Platform_Structure* i2c)
{
    IIC_GPIO_SET(&(i2c->SDA));
}

/**
 * @brief 将 SDA 引脚设置为低电平
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 */
static inline void SDA_RESET(I2C_Platform_Structure* i2c)
{
    IIC_GPIO_RESET(&(i2c->SDA));
}

/**
 * @brief 读取 SDA 引脚电平
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 引脚电平（0 或 1）
 */
static inline uint8_t SDA_Read(I2C_Platform_Structure* i2c)
{
    return IIC_GPIO_READ(&(i2c->SDA));
}

/*============================================================================*/
/*                         公共 API 实现                                      */
/*============================================================================*/

/**
 * @brief 注册一个 I2C 驱动实例
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] SDA 指向 SDA 引脚结构体的指针
 * @param[in] SCL 指向 SCL 引脚结构体的指针
 * @return 无
 */
void IIC_Register(I2C_Platform_Structure* i2c, Pin_Struct *SDA, Pin_Struct *SCL)
{
    /* 复制引脚配置 */
    i2c->SDA = *SDA;
    i2c->SCL = *SCL;

    /* 初始化引脚为空闲状态（高电平） */
    SCL_SET(i2c);
    SDA_SET(i2c);
}

/**
 * @brief 产生 I2C 起始信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 * @details 起始条件：SCL 为高时，SDA 由高变低
 */
void IIC_Start(I2C_Platform_Structure* i2c)
{
    SDA_SET(i2c);
    SCL_SET(i2c);
    IIC_DELAY();
    SDA_RESET(i2c);
    IIC_DELAY();
    SCL_RESET(i2c);
}

/**
 * @brief 产生 I2C 停止信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 无
 * @details 停止条件：SCL 为高时，SDA 由低变高
 */
void IIC_Stop(I2C_Platform_Structure* i2c)
{
    SCL_RESET(i2c);
    SDA_RESET(i2c);
    IIC_DELAY();
    SCL_SET(i2c);
    IIC_DELAY();
    SDA_SET(i2c);
    IIC_DELAY();
}

/**
 * @brief 等待 I2C 应答信号
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 应答状态
 * @retval 0 收到应答（ACK，成功）
 * @retval 1 收到非应答（NACK，失败）
 */
uint8_t IIC_Wait_Ack(I2C_Platform_Structure* i2c)
{
    uint8_t ack;

    SDA_SET(i2c);        /* 释放 SDA，让从机控制 */
    IIC_DELAY();
    SCL_SET(i2c);        /* 产生第 9 个时钟脉冲 */
    IIC_DELAY();
    ack = SDA_Read(i2c); /* 读取 ACK/NACK（0=ACK，1=NACK） */
    SCL_RESET(i2c);      /* 结束时钟脉冲 */
    IIC_DELAY();

    return ack;
}

/**
 * @brief 通过 I2C 发送一个字节
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Byte 要发送的数据字节
 * @return 无
 * @details 发送 8 位数据，高位在前（MSB first）
 */
void IIC_Send_Byte(I2C_Platform_Structure* i2c, uint8_t Byte)
{
    uint8_t i;

    SCL_RESET(i2c);

    for (i = 0; i < 8; i++) {
        /* 根据最高位设置 SDA */
        if ((Byte & 0x80) >> 7) {
            SDA_SET(i2c);
        } else {
            SDA_RESET(i2c);
        }

        Byte <<= 1;      /* 左移到下一位 */
        IIC_DELAY();
        SCL_SET(i2c);    /* 时钟脉冲 */
        IIC_DELAY();
        SCL_RESET(i2c);
        IIC_DELAY();
    }
}

/**
 * @brief 通过 I2C 读取一个字节
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] ack 读取后发送的应答/非应答标志
 * @return 接收到的数据字节
 * @details 读取 8 位数据，高位在前（MSB first）
 */
uint8_t IIC_Read_Byte(I2C_Platform_Structure* i2c, uint8_t ack)
{
    uint8_t i = 0;
    uint8_t Byte = 0x00;

    SDA_SET(i2c); /* 释放 SDA，让从机控制 */
    IIC_DELAY();

    /* 读取 8 位数据 */
    for (i = 0; i < 8; i++) {
        SCL_SET(i2c);
        IIC_DELAY();

        Byte <<= 1;
        if (SDA_Read(i2c)) {
            Byte |= 0x01;
        }

        SCL_RESET(i2c);
        IIC_DELAY();
    }

    /* 发送 ACK/NACK */
    if (ack) {
        SDA_RESET(i2c); /* 发送应答 ACK */
        SCL_SET(i2c);
        IIC_DELAY();
        SCL_RESET(i2c);
    } else {
        SDA_SET(i2c);   /* 发送非应答 NACK */
        SCL_SET(i2c);
        IIC_DELAY();
        SCL_RESET(i2c);
    }

    return Byte;
}

/**
 * @brief 扫描 I2C 总线上的设备
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @return 找到的设备数量
 * @details 扫描所有可能的 I2C 地址（0x00-0x7F）并打印找到的设备
 */
uint8_t IIC_Scan(I2C_Platform_Structure* i2c)
{
    uint8_t Find_Flag = 0;

    IIC_DEBUG_PRINT("\r\n==============================\r\n");

    for (uint8_t addr = 0; addr <= 127; addr++) {
        if (!(addr % 10) && addr != 0) {
            IIC_DEBUG_PRINT("\r\n");
        }

        IIC_Start(i2c);
        IIC_Send_Byte(i2c, addr << 1 | READ);

        if (IIC_Wait_Ack(i2c) == 0) {
            IIC_DEBUG_PRINT(" 0x%02X ", addr);
            Find_Flag++;
        } else {
            IIC_DEBUG_PRINT("  -   ");
        }

        IIC_Stop(i2c);
        IIC_DELAY();
    }

    IIC_DEBUG_PRINT("\r\n==============================\r\n");
    IIC_DEBUG_PRINT("找到设备总数: %d\r\n", Find_Flag);

    return Find_Flag;
}

/*============================================================================*/
/*                         寄存器操作                                          */
/*============================================================================*/

/**
 * @brief 从 I2C 设备寄存器读取数据
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Device_address I2C 设备地址（7位）
 * @param[in] Reg_address 要读取的寄存器地址
 * @param[in] Size 要读取的字节数
 * @param[out] Data_buf 存储读取数据的缓冲区
 * @return 来自 enum IIC_Error_Code 的错误码
 */
uint8_t IIC_Read_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,
    uint8_t Reg_address,
    uint8_t Size,
    uint8_t Data_buf[]
)
{
    uint8_t error_code = IIC_OK;

    /* 1. 发送起始条件 */
    IIC_Start(i2c);

    /* 2. 发送设备地址（写模式） */
    IIC_Send_Byte(i2c, (Device_address << 1) | WRITE);
    if (IIC_Wait_Ack(i2c) != 0) {
        IIC_Stop(i2c);
        error_code = IIC_Error_ACK_ADDR;
        return error_code;
    }

    /* 3. 发送寄存器地址 */
    IIC_Send_Byte(i2c, Reg_address);
    if (IIC_Wait_Ack(i2c) != 0) {
        IIC_Stop(i2c);
        error_code = IIC_Error_ACK_REG;
        return error_code;
    }

    /* 4. 发送重复起始条件 */
    IIC_Start(i2c);

    /* 5. 发送设备地址（读模式） */
    IIC_Send_Byte(i2c, Device_address << 1 | READ);
    if (IIC_Wait_Ack(i2c) != 0) {
        IIC_Stop(i2c);
        error_code = IIC_Error_ACK_ADDR;
        return error_code;
    }

    /* 6. 读取数据字节 */
    for (int i = 0; i < Size; i++) {
        /* 除最后一个字节外都发送 ACK，最后一个字节发送 NACK */
        Data_buf[i] = IIC_Read_Byte(i2c, (i == (Size - 1)) ? NO_ACK : ACK);
    }

    /* 7. 发送停止条件 */
    IIC_Stop(i2c);

    return IIC_OK;
}

/**
 * @brief 向 I2C 设备寄存器写入数据
 * @param[in] i2c 指向 I2C 平台结构体的指针
 * @param[in] Device_address I2C 设备地址（7位）
 * @param[in] Reg_address 要写入的寄存器地址
 * @param[in] Size 要写入的字节数
 * @param[in] Data_buf 指向数据缓冲区的指针
 * @return 来自 enum IIC_Error_Code 的错误码
 */
uint8_t IIC_Write_Reg(
    I2C_Platform_Structure* i2c,
    uint8_t Device_address,
    uint8_t Reg_address,
    uint8_t Size,
    const void* Data_buf
)
{
    uint8_t error_code = IIC_OK;

    /* 1. 发送起始条件 */
    IIC_Start(i2c);

    /* 2. 发送设备地址（写模式） */
    IIC_Send_Byte(i2c, (Device_address << 1) | WRITE);
    if (IIC_Wait_Ack(i2c) != 0) {
        IIC_Stop(i2c);
        error_code = IIC_Error_ACK_ADDR;
        return error_code;
    }

    /* 3. 发送寄存器地址 */
    IIC_Send_Byte(i2c, Reg_address);
    if (IIC_Wait_Ack(i2c) != 0) {
        IIC_Stop(i2c);
        error_code = IIC_Error_ACK_REG;
        return error_code;
    }

    /* 4. 写入数据字节 */
    const uint8_t* buf = (const uint8_t*)Data_buf;
    for (int i = 0; i < Size; i++) {
        IIC_Send_Byte(i2c, buf[i]);
        if (IIC_Wait_Ack(i2c) != 0) {
            IIC_Stop(i2c);
            error_code = IIC_Error_ACK_DATA;
            return error_code;
        }
    }

    /* 5. 发送停止条件 */
    IIC_Stop(i2c);

    return error_code;
}
