#include "aht30.h"
#include <stddef.h>

// ********** AHT30 Command Set ********** //
#define AHT30_INIT_COMMAND           0xE1    // 初始化命令
#define AHT30_TRIGGER_MEASUREMENT    0xAC    // 触发测量命令
#define AHT30_SOFT_RESET             0xBA    // 软复位命令

// ********** AHT30 Status Bits ********** //
#define AHT30_STATUS_BUSY            0x80    // 忙标志位
#define AHT30_STATUS_MODE            0x70    // 模式位
#define AHT30_STATUS_CALIBRATION     0x08    // 校准使能位

// ********** AHT30 Parameters ********** //
#define AHT30_INIT_DELAY_MS          10      // 初始化延迟时间
#define AHT30_MEASUREMENT_DELAY_MS   80      // 测量延迟时间

// ********** Device Driver Structure ********** //
typedef struct {
    I2C_Platform_Structure i2c;         // Bit-bang I2C driver
    uint8_t address;                    // 7-bit device address
    uint8_t initialized;                // 初始化标志
} AHT30_Device_Driver;

// ********** Static Variables ********** //
static AHT30_Device_Driver aht30_device_drivers[AHT30_MAX_NUM];

// ********** Static Functions ********** //
static AHT30_Device_Driver* get_driver(const AHT30_Handle* handler)
{
    if (handler == NULL || handler->ID >= AHT30_MAX_NUM) {
        return NULL;
    }
    if (aht30_device_drivers[handler->ID].address == 0) {
        return NULL;
    }
    return &aht30_device_drivers[handler->ID];
}

static AHT30_Status_Enum aht30_send_command(AHT30_Device_Driver* dev, uint8_t command, const uint8_t* params, uint8_t param_len)
{
    if (dev == NULL) {
        return AHT30_Status_Error;
    }
    
    // For AHT30, we need to send command followed by parameters
    uint8_t data[3] = {command, 0, 0};  // Max 3 bytes for command + parameters
    
    if (params != NULL && param_len > 0) {
        for (int i = 0; i < param_len && i < 2; i++) {
            data[i+1] = params[i];
        }
    }
    
    return (IIC_Write_Reg(&dev->i2c, dev->address, data[0], param_len, &data[1]) == IIC_OK)
        ? AHT30_Status_OK
        : AHT30_Status_Error;
}

static AHT30_Status_Enum aht30_read_data(AHT30_Device_Driver* dev, uint8_t* data, uint8_t len)
{
    if (dev == NULL || data == NULL || len == 0) {
        return AHT30_Status_Error;
    }
    
    // For AHT30, we read data directly from the device without specifying a register
    // So we pass 0 as register address (it's ignored by the sensor)
    return (IIC_Read_Reg(&dev->i2c, dev->address, 0, len, data) == IIC_OK)
        ? AHT30_Status_OK
        : AHT30_Status_Error;
}

// ********** Public Functions ********** //
AHT30_Status_Enum AHT30_Device_Create(AHT30_Handle* handler, Pin_Struct* SDA, Pin_Struct* SCL, uint8_t address)
{
    if (handler == NULL || SDA == NULL || SCL == NULL) {
        return AHT30_Status_Error;
    }
    
    // Use default address if not specified
    if (address == 0) {
        address = AHT30_ADDRESS;
    }

    for (int i = 0; i < AHT30_MAX_NUM; i++) {
        if (aht30_device_drivers[i].address == 0) {
            IIC_Register(&aht30_device_drivers[i].i2c, SDA, SCL);   // IIC 设备注册
            aht30_device_drivers[i].address = address;              // 绑定设备地址
            aht30_device_drivers[i].initialized = 0;                // 初始化标志清零
            handler->ID = (uint16_t)i;                              // 绑定设备ID
            return AHT30_Status_OK;
        }
    }
    return AHT30_Status_Error;
}

AHT30_Status_Enum AHT30_Init(AHT30_Handle* handler)
{
    AHT30_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return AHT30_Status_Error;
    }

    // Send initialization command
    uint8_t init_params[2] = {0x08, 0x00};
    if (aht30_send_command(dev, AHT30_INIT_COMMAND, init_params, 2) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    // Wait for initialization
    // Note: Using a simple loop delay since we don't have access to HAL_Delay here
    for (volatile int i = 0; i < 10000; i++);  // Simple delay
    
    // Check if initialization was successful
    uint8_t status;
    if (aht30_read_data(dev, &status, 1) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    // Check calibration bit
    if ((status & AHT30_STATUS_CALIBRATION) == 0) {
        return AHT30_Status_Error;
    }
    
    dev->initialized = 1;
    return AHT30_Status_OK;
}

AHT30_Status_Enum AHT30_Read_Data(AHT30_Handle* handler, float* temperature, float* humidity)
{
    AHT30_Device_Driver* dev = get_driver(handler);
    if (dev == NULL || temperature == NULL || humidity == NULL) {
        return AHT30_Status_Error;
    }
    
    if (!dev->initialized) {
        return AHT30_Status_Uninitialized;
    }
    
    // Trigger measurement
    uint8_t trigger_params[2] = {0x33, 0x00};
    if (aht30_send_command(dev, AHT30_TRIGGER_MEASUREMENT, trigger_params, 2) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    // Wait for measurement to complete
    for (volatile int i = 0; i < 80000; i++);  // Simple delay for ~80ms
    
    // Read status and data
    uint8_t data[6];
    if (aht30_read_data(dev, data, 6) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    // Check if device is busy
    if ((data[0] & AHT30_STATUS_BUSY) != 0) {
        return AHT30_Status_Busy;
    }
    
    // Parse humidity data (20 bits)
    uint32_t hum_raw = ((uint32_t)data[1] << 12) | ((uint32_t)data[2] << 4) | ((uint32_t)data[3] >> 4);
    *humidity = (float)hum_raw * 100.0f / 1048576.0f;  // 1048576 = 2^20
    
    // Parse temperature data (20 bits)
    uint32_t temp_raw = (((uint32_t)data[3] & 0x0F) << 16) | ((uint32_t)data[4] << 8) | (uint32_t)data[5];
    *temperature = (float)temp_raw * 200.0f / 1048576.0f - 50.0f;
    
    return AHT30_Status_OK;
}

AHT30_Status_Enum AHT30_Reset(AHT30_Handle* handler)
{
    AHT30_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return AHT30_Status_Error;
    }
    
    // Send soft reset command
    if (aht30_send_command(dev, AHT30_SOFT_RESET, NULL, 0) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    // Wait for reset to complete
    for (volatile int i = 0; i < 20000; i++);  // Simple delay
    
    // Mark as uninitialized
    dev->initialized = 0;
    
    return AHT30_Status_OK;
}

AHT30_Status_Enum AHT30_Get_Status(AHT30_Handle* handler)
{
    AHT30_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return AHT30_Status_Error;
    }
    
    uint8_t status;
    if (aht30_read_data(dev, &status, 1) != AHT30_Status_OK) {
        return AHT30_Status_Error;
    }
    
    if ((status & AHT30_STATUS_BUSY) != 0) {
        return AHT30_Status_Busy;
    }
    
    if ((status & AHT30_STATUS_CALIBRATION) == 0) {
        return AHT30_Status_Uninitialized;
    }
    
    return AHT30_Status_OK;
}