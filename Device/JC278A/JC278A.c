#include "main.h"  // 必须先包含 main.h，确保 UART_HandleTypeDef 被定义
#include "JC278A.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>

// ********** Device Driver Structure ********** //
typedef struct {
    uint16_t    nodeId;     //模块ID
    uint16_t    netId;      //网络ID
    uint8_t     rfPower;    //射频功率
    uint8_t     baudRate;   //波特率
    uint8_t     rfChannel;  //射频通道
    uint8_t     spaceRate;  //空间速率
    uint8_t     dataLength; //数据长度
    uint8_t     checkSum;   //校验和
} JC278A_Device_configure;
typedef struct {
    UART_HandleTypeDef* uart;            // UART句柄指针
    uint8_t rx_buffer[JC278A_RX_BUFFER_SIZE];  // 接收缓冲区
    uint16_t rx_length;                   // 接收数据长度
    uint8_t initialized;                  // 初始化标志
    JC278A_Device_configure configure;    // 设备配置
} JC278A_Device_Driver;

// ********** Static Variables ********** //
static JC278A_Device_Driver jc278a_device_drivers[JC278A_MAX_NUM];

// ********** Static Functions ********** //
static JC278A_Device_Driver* get_driver(const JC278A_Handle* handler)
{
    if (handler == NULL || handler->ID >= JC278A_MAX_NUM) {
        return NULL;
    }
    if (jc278a_device_drivers[handler->ID].uart == NULL) {
        return NULL;
    }
    return &jc278a_device_drivers[handler->ID];
}

static JC278A_Status_Enum jc278a_send_bytes(JC278A_Device_Driver* dev, const uint8_t* data, uint16_t length, uint32_t timeout)
{
    if (dev == NULL || data == NULL || length == 0) {
        return JC278A_Status_Error;
    }
    
    if (dev->uart == NULL) {
        return JC278A_Status_Error;
    }
    
    HAL_StatusTypeDef hal_status = HAL_UART_Transmit(dev->uart, (uint8_t*)data, length, 
                                                      (timeout == 0) ? JC278A_DEFAULT_TIMEOUT : timeout);
    
    if (hal_status == HAL_OK) {
        return JC278A_Status_OK;
    } else if (hal_status == HAL_TIMEOUT) {
        return JC278A_Status_Timeout;
    } else {
        return JC278A_Status_Error;
    }
}

static JC278A_Status_Enum jc278a_receive_bytes(JC278A_Device_Driver* dev, uint8_t* data, uint16_t* length, uint32_t timeout)
{
    if (dev == NULL || data == NULL || length == NULL || *length == 0) {
        return JC278A_Status_Error;
    }
    
    if (dev->uart == NULL) {
        return JC278A_Status_Error;
    }
    
    uint16_t expected_len = *length;
    HAL_StatusTypeDef hal_status = HAL_UART_Receive(dev->uart, data, expected_len, 
                                                     (timeout == 0) ? JC278A_DEFAULT_TIMEOUT : timeout);
    
    if (hal_status == HAL_OK) {
        *length = expected_len;
        return JC278A_Status_OK;
    } else if (hal_status == HAL_TIMEOUT) {
        *length = 0;
        return JC278A_Status_Timeout;
    } else {
        *length = 0;
        return JC278A_Status_Error;
    }
}

// ********** Public Functions ********** //

JC278A_Status_Enum JC278A_Device_Detection(JC278A_Handle* handler)
{
    if (handler == NULL || handler->ID >= JC278A_MAX_NUM) {
        return JC278A_Status_Error;
    }
    
    JC278A_Device_Driver* dev = &jc278a_device_drivers[handler->ID];
    if (dev->uart == NULL) {
        return JC278A_Status_Error;
    }
    
    // 简单的检测：检查UART是否已初始化
    // 可以根据实际需求添加更复杂的检测逻辑，如发送AT命令检测响应
    return JC278A_Status_OK;
}

JC278A_Status_Enum JC278A_Device_Create(JC278A_Handle* handler, UART_HandleTypeDef* uart_handle)
{
    if (handler == NULL || uart_handle == NULL) {
        return JC278A_Status_Error;
    }

    // 查找空闲的设备槽
    for (int i = 0; i < JC278A_MAX_NUM; i++) {
        if (jc278a_device_drivers[i].uart == NULL) {
            jc278a_device_drivers[i].uart = uart_handle;
            jc278a_device_drivers[i].rx_length = 0;
            jc278a_device_drivers[i].initialized = 0;
            memset(jc278a_device_drivers[i].rx_buffer, 0, JC278A_RX_BUFFER_SIZE);
            handler->ID = (uint16_t)i;
            return JC278A_Status_OK;
        }
    }
    return JC278A_Status_Error;
}

JC278A_Status_Enum JC278A_Init(JC278A_Handle* handler)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return JC278A_Status_Error;
    }

    // 清空接收缓冲区
    memset(dev->rx_buffer, 0, JC278A_RX_BUFFER_SIZE);
    dev->rx_length = 0;
    
    // 可以在这里添加特定的初始化命令
    // 例如：发送复位命令、配置参数等
    // 根据JC278A的实际协议进行初始化
    
    dev->initialized = 1;
    return JC278A_Status_OK;
}

JC278A_Status_Enum JC278A_Send_Data(JC278A_Handle* handler, const uint8_t* data, uint16_t length, uint32_t timeout)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return JC278A_Status_Error;
    }
    
    return jc278a_send_bytes(dev, data, length, timeout);
}

JC278A_Status_Enum JC278A_Receive_Data(JC278A_Handle* handler, uint8_t* data, uint16_t* length, uint32_t timeout)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return JC278A_Status_Error;
    }
    
    return jc278a_receive_bytes(dev, data, length, timeout);
}

JC278A_Status_Enum JC278A_Send_Command(JC278A_Handle* handler, const uint8_t* cmd, uint16_t cmd_len, 
                                       uint8_t* response, uint16_t* response_len, uint32_t timeout)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL || cmd == NULL || cmd_len == 0) {
        return JC278A_Status_Error;
    }
    
    // 发送命令
    JC278A_Status_Enum status = jc278a_send_bytes(dev, cmd, cmd_len, timeout);
    if (status != JC278A_Status_OK) {
        return status;
    }
    
    // 如果有响应缓冲区，则接收响应
    if (response != NULL && response_len != NULL && *response_len > 0) {
        uint32_t recv_timeout = (timeout == 0) ? JC278A_DEFAULT_TIMEOUT : timeout;
        status = jc278a_receive_bytes(dev, response, response_len, recv_timeout);
    }
    
    return status;
}

JC278A_Status_Enum JC278A_Reset(JC278A_Handle* handler)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return JC278A_Status_Error;
    }
    
    // 根据JC278A协议发送复位命令
    // 这里使用通用的复位命令格式，实际使用时需要根据JC278A手册修改
    uint8_t reset_cmd[] = {0xAA, 0x55, 0x01, 0x00};  // 示例命令，需要根据实际协议修改
    uint8_t response[32];
    uint16_t response_len = sizeof(response);
    
    JC278A_Status_Enum status = JC278A_Send_Command(handler, reset_cmd, sizeof(reset_cmd), 
                                                     response, &response_len, 1000);
    
    if (status == JC278A_Status_OK) {
        dev->initialized = 0;  // 复位后需要重新初始化
    }
    
    return status;
}

JC278A_Status_Enum JC278A_Set_Mode(JC278A_Handle* handler, uint8_t mode)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL) {
        return JC278A_Status_Error;
    }
    
    // 根据JC278A协议发送设置模式命令
    // 这里使用通用的命令格式，实际使用时需要根据JC278A手册修改
    uint8_t mode_cmd[] = {0xAA, 0x55, 0x02, mode};  // 示例命令，需要根据实际协议修改
    uint8_t response[32];
    uint16_t response_len = sizeof(response);
    
    return JC278A_Send_Command(handler, mode_cmd, sizeof(mode_cmd), 
                              response, &response_len, 1000);
}

JC278A_Status_Enum JC278A_Get_Info(JC278A_Handle* handler, uint8_t* info, uint16_t* info_len)
{
    JC278A_Device_Driver* dev = get_driver(handler);
    if (dev == NULL || info == NULL || info_len == NULL || *info_len == 0) {
        return JC278A_Status_Error;
    }
    
    // 根据JC278A协议发送获取信息命令
    // 这里使用通用的命令格式，实际使用时需要根据JC278A手册修改
    uint8_t info_cmd[] = {0xAA, 0x55, 0x03, 0x00};  // 示例命令，需要根据实际协议修改
    
    return JC278A_Send_Command(handler, info_cmd, sizeof(info_cmd), 
                              info, info_len, 1000);
}

