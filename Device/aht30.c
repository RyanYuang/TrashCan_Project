/*
 * aht30.c
 *
 *  Created on: Nov 28, 2025
 *      Author: zeng
 */
#include "aht30.h"

#define AHT30_ADDRESS 0x70
/**
 * @brief 初始化AHT30
 */
void AHT30_Init(void)
{
    uint8_t readBuffer;
    HAL_Delay(40);
    HAL_I2C_Master_Receive(&hi2c1, AHT30_ADDRESS, &readBuffer, 1, HAL_MAX_DELAY);
    if((readBuffer & 0x08) == 0x00)
    {
        uint8_t sendBuffer[3] = {0xBE, 0x08, 0x00};
        HAL_I2C_Master_Transmit(&hi2c1, AHT30_ADDRESS, sendBuffer, 3, HAL_MAX_DELAY);
    }
}

/**
 * @brief 读取AHT30的温湿度数据
 * @param Temperature 指向温度值的指针
 * @param Humidity 指向湿度值的指针
 */
void AHT30_Read(float *Temperature, float *Humidity)
{
    uint8_t sendbuffer[3] = {0xAC, 0x33, 0x00};
    uint8_t readbuffer[6];

    HAL_I2C_Master_Transmit(&hi2c1, AHT30_ADDRESS, sendbuffer, 3, HAL_MAX_DELAY);
    HAL_Delay(75);
    HAL_I2C_Master_Receive(&hi2c1, AHT30_ADDRESS, readbuffer, 6, HAL_MAX_DELAY);

    if ((readbuffer[0] & 0x80) == 0x00)
    {
        uint32_t data = 0;
        data = ((uint32_t)readbuffer[3] >> 4) + ((uint32_t)readbuffer[2] << 4) + ((uint32_t)readbuffer[1] << 12);
        *Humidity = data * 100.0f / (1 << 20);

        data = (((uint32_t)readbuffer[3] & 0x0F) << 16) + ((uint32_t)readbuffer[4] << 8) + (uint32_t)readbuffer[5];
        *Temperature = data * 200.0f / (1 << 20) - 50;
    }
}

/**
 * @brief 初始化AHT302
 */
void AHT302_Init(void)
{
    uint8_t readBuffer;
    HAL_Delay(40);
    HAL_I2C_Master_Receive(&hi2c2, AHT30_ADDRESS, &readBuffer, 1, HAL_MAX_DELAY);
    if((readBuffer & 0x08) == 0x00)
    {
        uint8_t sendBuffer[3] = {0xBE, 0x08, 0x00};
        HAL_I2C_Master_Transmit(&hi2c2, AHT30_ADDRESS, sendBuffer, 3, HAL_MAX_DELAY);
    }
}

/**
 * @brief 读取AHT302的温湿度数据
 * @param Temperature 指向温度值的指针
 * @param Humidity 指向湿度值的指针
 */
void AHT302_Read(float *Temperature, float *Humidity)
{
    uint8_t sendbuffer[3] = {0xAC, 0x33, 0x00};
    uint8_t readbuffer[6];

    HAL_I2C_Master_Transmit(&hi2c2, AHT30_ADDRESS, sendbuffer, 3, HAL_MAX_DELAY);
    HAL_Delay(75);
    HAL_I2C_Master_Receive(&hi2c2, AHT30_ADDRESS, readbuffer, 6, HAL_MAX_DELAY);

    if ((readbuffer[0] & 0x80) == 0x00)
    {
        uint32_t data = 0;
        data = ((uint32_t)readbuffer[3] >> 4) + ((uint32_t)readbuffer[2] << 4) + ((uint32_t)readbuffer[1] << 12);
        *Humidity = data * 100.0f / (1 << 20);

        data = (((uint32_t)readbuffer[3] & 0x0F) << 16) + ((uint32_t)readbuffer[4] << 8) + (uint32_t)readbuffer[5];
        *Temperature = data * 200.0f / (1 << 20) - 50;
    }
}


