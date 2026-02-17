/**
  ******************************************************************************
  * @file           : protocol_parser.c
  * @brief          : 串口协议解析模块实现
  * @author         : Generated for Environmental Monitoring Car
  * @date           : 2026-02-14
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "protocol_parser.h"
#include "uart_protocol.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

// 调试日志开关（注释掉这行可关闭调试输出）
//#define PROTOCOL_DEBUG_ENABLE

#ifdef PROTOCOL_DEBUG_ENABLE
    #define DEBUG_LOG(fmt, ...) printf(fmt, ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...)
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

// 最新接收到的控制指令
static ControlCommand_t g_LastCommand = CMD_INVALID;

// 回调函数指针
static CommandCallback_t g_CommandCallback = NULL;
static ThresholdCallback_t g_ThresholdCallback = NULL;

/* Private function prototypes -----------------------------------------------*/
static ParseResult_t Parse_ControlCommand(const char *frame);
static ParseResult_t Parse_ThresholdConfig(const char *frame);
static float ParseFloat(const char **str);

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 手动解析浮点数（不依赖sscanf）
 * @param str 指向字符串指针的指针，解析后会更新位置
 * @retval 解析出的浮点数
 */
static float ParseFloat(const char **str)
{
    const char *p = *str;
    float result = 0.0f;
    float sign = 1.0f;
    float decimal = 0.0f;
    int decimal_places = 0;
    
    // Skip spaces
    while (*p == ' ') p++;
    
    // Handle sign
    if (*p == '-') {
        sign = -1.0f;
        p++;
    } else if (*p == '+') {
        p++;
    }
    
    // Parse integer part
    while (isdigit(*p)) {
        result = result * 10.0f + (*p - '0');
        p++;
    }
    
    // Parse decimal part
    if (*p == '.') {
        p++;
        while (isdigit(*p)) {
            decimal = decimal * 10.0f + (*p - '0');
            decimal_places++;
            p++;
        }
    }
    
    // Combine result
    result = result + decimal / pow(10.0f, decimal_places);
    result *= sign;
    
    *str = p;
    return result;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief 解析控制指令帧：@<指令码>\r\n
 * @param frame 协议帧字符串
 * @retval 解析结果
 */
static ParseResult_t Parse_ControlCommand(const char *frame)
{
    if (frame == NULL || frame[0] != '@') {
        return PARSE_ERROR;
    }
    
    // 跳过'@'字符
    const char *cmdStr = &frame[1];
    
    // 跳过前导空格
    while (*cmdStr == ' ') {
        cmdStr++;
    }
    
    // 解析指令码
    if (!isdigit(*cmdStr)) {
        return PARSE_ERROR;
    }
    
    int cmdValue = atoi(cmdStr);
    printf("cmdValue:%d\r\n",cmdValue);
    
    // 验证指令码范围
    if (cmdValue < 0 || cmdValue > 8) {
        g_LastCommand = CMD_INVALID;
        return PARSE_ERROR;
    }
    
    g_LastCommand = (ControlCommand_t)cmdValue;
    
    // 调用回调函数
    if (g_CommandCallback != NULL) {
        g_CommandCallback(g_LastCommand);
    }
    
    return PARSE_OK;
}

/**
 * @brief 解析阈值配置帧：#T<f>,<f>,H<f>,<f>,G<f>,<f>,L<f>,<f>\r\n
 * @param frame 协议帧字符串
 * @retval 解析结果
 */
static ParseResult_t Parse_ThresholdConfig(const char *frame)
{
    if (frame == NULL || frame[0] != '#') {
        DEBUG_LOG("[ThresholdParse] Error: Frame is NULL or first char is not #\r\n");
        return PARSE_ERROR;
    }
    
    // Print raw data received
    DEBUG_LOG("[ThresholdParse] Raw: %s\r\n", frame);
    DEBUG_LOG("[ThresholdParse] Length: %d\r\n", strlen(frame));
    
    ThresholdConfig_t newConfig;
    
    // Try parsing with more flexible format (allow spaces)
    int result = sscanf(frame, "#T%f,%f,H%f,%f,G%f,%f,L%f,%f",
                       &newConfig.temp_low,
                       &newConfig.temp_high,
                       &newConfig.hum_low,
                       &newConfig.hum_high,
                       &newConfig.co_warning,
                       &newConfig.co_danger,
                       &newConfig.light_low,
                       &newConfig.light_high);
    
    // Print matching result
    DEBUG_LOG("[ThresholdParse] Matched fields: %d/8\r\n", result);
    
    // If sscanf failed, try manual parsing
    if (result != 8) {
        DEBUG_LOG("[ThresholdParse] sscanf failed, trying manual parse...\r\n");
        
        // Print hex dump of first 20 chars to check for hidden characters
        DEBUG_LOG("[ThresholdParse] Hex dump: ");
        for (int i = 0; i < 20 && frame[i] != '\0'; i++) {
            DEBUG_LOG("%02X ", (unsigned char)frame[i]);
        }
        DEBUG_LOG("\r\n");
        
        // Manual parsing: find each letter marker and parse the values after it
        const char *p = frame;
        
        // Skip '#T'
        if (*p == '#') p++;
        if (*p == 'T') p++;
        
        DEBUG_LOG("[ThresholdParse] After skipping #T, remaining: '%s'\r\n", p);
        
        // Parse temperature low and high
        float temp_l, temp_h;
        int n = sscanf(p, "%f,%f", &temp_l, &temp_h);
        DEBUG_LOG("[ThresholdParse] sscanf returned %d, temp_l=%.1f, temp_h=%.1f\r\n", n, temp_l, temp_h);
        
        if (n != 2) {
            DEBUG_LOG("[ThresholdParse] sscanf failed, using manual float parser...\r\n");
            // Use manual parser
            newConfig.temp_low = ParseFloat(&p);
            if (*p == ',') p++;  // Skip comma
            newConfig.temp_high = ParseFloat(&p);
            DEBUG_LOG("[ThresholdParse] Manual: temp_low=%.1f, temp_high=%.1f\r\n", 
                   newConfig.temp_low, newConfig.temp_high);
        } else {
            newConfig.temp_low = temp_l;
            newConfig.temp_high = temp_h;
        }
        
        // Find 'H'
        p = strchr(p, 'H');
        if (!p) {
            DEBUG_LOG("[ThresholdParse] 'H' marker not found\r\n");
            goto parse_failed;
        }
        p++; // Skip 'H'
        
        // Parse humidity low and high
        newConfig.hum_low = ParseFloat(&p);
        if (*p == ',') p++;
        newConfig.hum_high = ParseFloat(&p);
        DEBUG_LOG("[ThresholdParse] Humidity: %.1f ~ %.1f\r\n", newConfig.hum_low, newConfig.hum_high);
        
        // Find 'G'
        p = strchr(p, 'G');
        if (!p) {
            DEBUG_LOG("[ThresholdParse] 'G' marker not found\r\n");
            goto parse_failed;
        }
        p++; // Skip 'G'
        
        // Parse CO warning and danger
        newConfig.co_warning = ParseFloat(&p);
        if (*p == ',') p++;
        newConfig.co_danger = ParseFloat(&p);
        DEBUG_LOG("[ThresholdParse] CO: %.1f ~ %.1f\r\n", newConfig.co_warning, newConfig.co_danger);
        
        // Find 'L'
        p = strchr(p, 'L');
        if (!p) {
            DEBUG_LOG("[ThresholdParse] 'L' marker not found\r\n");
            goto parse_failed;
        }
        p++; // Skip 'L'
        
        // Parse light low and high
        newConfig.light_low = ParseFloat(&p);
        if (*p == ',') p++;
        newConfig.light_high = ParseFloat(&p);
        DEBUG_LOG("[ThresholdParse] Light: %.1f ~ %.1f\r\n", newConfig.light_low, newConfig.light_high);
        
        DEBUG_LOG("[ThresholdParse] Manual parse successful!\r\n");
    }
    
    // Print parsed values
    DEBUG_LOG("[ThresholdParse] Parsed values:\r\n");
    DEBUG_LOG("  Temp: %.1f ~ %.1f\r\n", newConfig.temp_low, newConfig.temp_high);
    DEBUG_LOG("  Humidity: %.1f ~ %.1f\r\n", newConfig.hum_low, newConfig.hum_high);
    DEBUG_LOG("  CO: Warn%.1f Danger%.1f\r\n", newConfig.co_warning, newConfig.co_danger);
    DEBUG_LOG("  Light: %.1f ~ %.1f\r\n", newConfig.light_low, newConfig.light_high);
    
    // Validate parameters
    if (newConfig.temp_low >= newConfig.temp_high ||
        newConfig.hum_low >= newConfig.hum_high ||
        newConfig.co_warning >= newConfig.co_danger ||
        newConfig.light_low >= newConfig.light_high) {
        DEBUG_LOG("[ThresholdParse] Error: Invalid params (low should < high)\r\n");
        // 发送错误消息给上位机
        HAL_UART_Transmit(&huart2, (uint8_t*)"#ERR:Invalid params\r\n", 21, 100);
        return PARSE_ERROR;
    }
    
    // Apply new configuration
    DEBUG_LOG("[ThresholdParse] Config applied!\r\n");
    ThresholdConfig_Apply(&newConfig);
    
    // 发送成功确认消息给上位机
    HAL_UART_Transmit(&huart2, (uint8_t*)"#OK:Config synced\r\n", 20, 100);
    
    // Call callback function
    if (g_ThresholdCallback != NULL) {
        g_ThresholdCallback(&newConfig);
    }
    
    return PARSE_OK;

parse_failed:
    DEBUG_LOG("[ThresholdParse] Parse failed!\r\n");
    DEBUG_LOG("[ThresholdParse] Format: #T<temp_low>,<temp_high>,H<hum_low>,<hum_high>,G<co_warn>,<co_danger>,L<light_low>,<light_high>\r\n");
    DEBUG_LOG("[ThresholdParse] Example: #T-10.0,45.0,H20.0,90.0,G30.0,50.0,L0.0,10000.0\r\n");
    // 发送解析失败消息给上位机
    HAL_UART_Transmit(&huart2, (uint8_t*)"#ERR:Parse failed\r\n", 20, 100);
    return PARSE_ERROR;
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief 初始化协议解析模块
 */
void Protocol_Parser_Init(void)
{
    g_LastCommand = CMD_INVALID;
    g_CommandCallback = NULL;
    g_ThresholdCallback = NULL;
}

/**
 * @brief 注册控制指令回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterCommandCallback(CommandCallback_t callback)
{
    g_CommandCallback = callback;
}

/**
 * @brief 注册阈值配置回调函数
 * @param callback 回调函数指针
 */
void Protocol_Parser_RegisterThresholdCallback(ThresholdCallback_t callback)
{
    g_ThresholdCallback = callback;
}

/**
 * @brief 解析接收到的协议帧
 * @param frame 协议帧字符串（以\0结尾）
 * @retval 解析结果
 */
ParseResult_t Protocol_Parse(const char *frame)
{
    if (frame == NULL || frame[0] == '\0') {
        return PARSE_ERROR;
    }
    
    // 根据首字符分发到对应的解析函数
    switch (frame[0]) {
        case '@':
            // 控制指令帧
            return Parse_ControlCommand(frame);
            
        case '#':
            // 阈值配置帧
        	DEBUG_LOG("[Protocol] Threshold config frame\r\n");
            return Parse_ThresholdConfig(frame);
            
        default:
            // 未知帧类型
            return PARSE_UNKNOWN_TYPE;
    }
}

/**
 * @brief 获取最新的控制指令
 * @retval 控制指令
 */
ControlCommand_t Protocol_GetLastCommand(void)
{
    return g_LastCommand;
}

/**
 * @brief 清除最新的控制指令（标记为已处理）
 */
void Protocol_ClearLastCommand(void)
{
    g_LastCommand = CMD_INVALID;
}

/**
 * @brief 获取控制指令的字符串描述
 * @param cmd 控制指令
 * @return 指令字符串描述
 */
const char* Protocol_GetCommandString(ControlCommand_t cmd)
{
    switch (cmd) {
        case CMD_STOP:
            return "Stop";
        case CMD_FORWARD:
            return "Forward";
        case CMD_BACKWARD:
            return "Backward";
        case CMD_TURN_LEFT:
            return "Turn Left";
        case CMD_TURN_RIGHT:
            return "Turn Right";
        case CMD_SPEED_25:
            return "Speed 25%";
        case CMD_SPEED_50:
            return "Speed 50%";
        case CMD_SPEED_75:
            return "Speed 75%";
        case CMD_SPEED_100:
            return "Speed 100%";
        default:
            return "Invalid";
    }
}
