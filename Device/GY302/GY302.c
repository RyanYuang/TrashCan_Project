#include "GY302.h"
#include <stddef.h>
#include <stdint.h>



// ********** Device Driver Structure ********** //
typedef struct {
    I2C_Platform_Structure i2c;         // Bit-bang I2C driver
    uint8_t address;                    // 7-bit device address
} GY302_Device_Driver;

// ********** Static Variables ********** //
static GY302_Device_Driver gy302_device_drivers[GY302_MAX_NUM];

// ********** Static Functions ********** //
static GY302_Device_Driver* get_driver(const GY302_Handle* handler)
{
    if (handler == NULL || handler->ID >= GY302_MAX_NUM) {
        return NULL;
    }
    if (gy302_device_drivers[handler->ID].address == 0) {
        return NULL;
    }
    return &gy302_device_drivers[handler->ID];
}

static GY302_Status_Enum gy302_send_command(GY302_Device_Driver* dev, uint8_t command)
{
    if (dev == NULL) {
        return GY302_Status_Error;
    }
    
    return (IIC_Write_Reg(&dev->i2c, dev->address, command, 0, NULL) == IIC_OK)
        ? GY302_Status_OK
        : GY302_Status_Error;
}

static GY302_Status_Enum gy302_read_data(GY302_Device_Driver* dev, uint8_t* data, uint8_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return GY302_Status_Error;
    }
    uint8_t res = GY302_Status_OK;
    
    // For BH1750, we read data directly from the device without specifying a register
    // The IIC_Read_Reg function is not suitable here.
    // So we implement the read process manually.
    IIC_Start(&dev->i2c);

    IIC_Send_Byte(&dev->i2c, (dev->address << 1) | 1); // Write address
    if (!IIC_Wait_Ack(&dev->i2c)) {
        IIC_Stop(&dev->i2c);
    }
    for(int i = 0; i < len; i++) {
        if (i == (len - 1)) {
            data[i] = IIC_Read_Byte(&dev->i2c, NO_ACK); // Last byte
        } else {
            data[i] = IIC_Read_Byte(&dev->i2c, ACK);    // Other bytes
        }
    }
    IIC_Stop(&dev->i2c);


    return res;
}

// ********** Public Functions ********** //

GY302_Status_Enum GY302_Device_Detection(GY302_Handle* handler)
{
	return (IIC_Scan(&gy302_device_drivers[handler->ID].i2c) > 0)? GY302_Status_OK : GY302_Status_Error;
}

GY302_Status_Enum GY302_Device_Create(GY302_Handle* handler, Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address)
{
    if (handler == NULL || SDA == NULL || SCL == NULL || address == 0) {
        return GY302_Status_Error;
    }

    for (int i = 0; i < GY302_MAX_NUM; i++) {
        if (gy302_device_drivers[i].address == 0) {
            IIC_Register(&gy302_device_drivers[i].i2c, SDA, SCL);   // IIC 设备注册
            gy302_device_drivers[i].address = address;              // 绑定设备地址
            handler->ID = (uint16_t)i;                              // 绑定设备ID
            return GY302_Status_OK;
        }
    }
    return GY302_Status_Error;
}

GY302_Status_Enum GY302_Init(GY302_Handle* handler)
{
    GY302_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return GY302_Status_Error;
    }

    // Power on the device
    if (gy302_send_command(dev, GY302_POWER_ON) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    
    // Reset the device
    if (gy302_send_command(dev, GY302_RESET) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    
    // Set to continuous high resolution mode
    if (gy302_send_command(dev, GY302_CONTINUOUS_H_RES_MODE) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    
    return GY302_Status_OK;
}

GY302_Status_Enum GY302_Read_Lux(GY302_Handle* handler, float* lux)
{
    GY302_Device_Driver* dev = get_driver(handler);
    if (dev == NULL || lux == NULL) {
        return GY302_Status_Error;
    }
    
    uint8_t data[2];
    if (gy302_read_data(dev, data, 2) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    printf("0X%X,0X%X\r\n",data[0],data[1]);
    
    // Calculate lux value
    uint16_t raw_value = (data[0] << 8) | data[1];
    *lux = (float)raw_value / 1.2f;  // Convert to lux
    
    return GY302_Status_OK;
}

GY302_Status_Enum GY302_Set_Mode(GY302_Handle* handler, uint8_t mode)
{
    GY302_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return GY302_Status_Error;
    }
    
    uint8_t command;
    switch (mode) {
        case 0:
            command = GY302_POWER_DOWN;
            break;
        case 1:
            command = GY302_CONTINUOUS_H_RES_MODE;
            break;
        case 2:
            command = GY302_CONTINUOUS_H_RES_MODE2;
            break;
        case 3:
            command = GY302_CONTINUOUS_L_RES_MODE;
            break;
        case 4:
            command = GY302_ONE_TIME_H_RES_MODE;
            break;
        case 5:
            command = GY302_ONE_TIME_H_RES_MODE2;
            break;
        case 6:
            command = GY302_ONE_TIME_L_RES_MODE;
            break;
        default:
            return GY302_Status_Error;
    }
    
    return (gy302_send_command(dev, command) == GY302_Status_OK)
        ? GY302_Status_OK
        : GY302_Status_Error;
}

GY302_Status_Enum GY302_Set_Measurement_Time(GY302_Handle* handler, uint8_t time)
{
    GY302_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return GY302_Status_Error;
    }
    
    // The measurement time is set by sending two commands:
    // High nibble: 01000xxx where xxx is bits 4-6 of time
    // Low nibble:  011xxxxx where xxxxx is bits 0-4 of time
    
    uint8_t high_cmd = GY302_CHANGE_MEASUREMENT_TIME_H | ((time >> 5) & 0x07);
    uint8_t low_cmd = GY302_CHANGE_MEASUREMENT_TIME_L | (time & 0x1F);
    
    if (gy302_send_command(dev, high_cmd) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    
    if (gy302_send_command(dev, low_cmd) != GY302_Status_OK) {
        return GY302_Status_Error;
    }
    
    return GY302_Status_OK;
}

GY302_Status_Enum GY302_Reset(GY302_Handle* handler)
{
    GY302_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return GY302_Status_Error;
    }
    
    return (gy302_send_command(dev, GY302_RESET) == GY302_Status_OK)
        ? GY302_Status_OK
        : GY302_Status_Error;
}
