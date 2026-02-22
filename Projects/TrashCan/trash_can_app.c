/**
 ******************************************************************************
 * @file    trash_can_app.c
 * @brief   垃圾桶项目应用实现（基于平台框架的示例）
 * @note    这是使用新平台框架重构后的应用代码示例
 ******************************************************************************
 */

#include "trash_can_app.h"
#include "platform.h"
#include "Devices.h"
#include "Drivers.h"
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>

// 串口协议模块
#include "uart_protocol.h"
#include "protocol_parser.h"
#include "threshold_config.h"
#include "MQ-2.h"

/* ========================= 私有变量 ========================= */
// 设备句柄
static OLED_Handle oled1;
static GY302_Handle gy302;
static AHT30_Handle aht30;

// 传感器数据
static float gy302_lux = 0;
static float mq2_ppm = 0;
static float temperature = 0.0f;
static float humidity = 0.0f;
static uint32_t adc_buffer[1];
static uint8_t oled_buffer[32];

// 控制数据
static uint8_t direction = 0;
static uint8_t speed = 0;
static int tb6612_speed = 0;
static int tb6612_speed_diff = 0;
static int tb6612_dir = 0;

// 数据上报标志
extern volatile uint8_t g_DataReportFlag;

/* ========================= 私有函数声明 ========================= */
static bool InitDevices(void);
static void SensorDataProcess(void);
static void DisplayUpdate(void);
static void ThresholdCheck(void);
static void MotorControl(void);
static void OnCommandReceived(ControlCommand_t cmd);

/* ========================= 应用生命周期实现 ========================= */

/**
 * @brief 应用初始化
 */
bool TrashCan_OnInit(void)
{
    printf("[TrashCan] Application initializing...\r\n");
    
    // 初始化平台
    if(!Platform_Init()) {
        printf("[TrashCan] Platform initialization failed\r\n");
        return false;
    }
    
    // 打印平台信息
    Platform_PrintInfo();
    
    // 初始化阈值配置
    ThresholdConfig_Init(NULL);
    
    // 初始化协议解析器
    Protocol_Parser_Init();
    Protocol_Parser_RegisterCommandCallback(OnCommandReceived);
    
    // 初始化串口协议
    UART_Protocol_Init(&huart2);
    
    // 初始化所有设备
    if(!InitDevices()) {
        printf("[TrashCan] Device initialization failed\r\n");
        return false;
    }
    
    // 启动定时器（数据上报周期）
    HAL_TIM_Base_Start_IT(&htim6);
    
    printf("[TrashCan] Application initialized successfully\r\n");
    return true;
}

/**
 * @brief 应用启动
 */
void TrashCan_OnStart(void)
{
    printf("[TrashCan] Application started\r\n");
    OLED_Clear(&oled1);
    OLED_ShowString(&oled1, 0, 0, "Gas Car", 16);
}

/**
 * @brief 应用主循环
 */
void TrashCan_OnLoop(void)
{
    // 处理传感器数据
    SensorDataProcess();
    
    // 检查是否需要上报数据和更新显示
    if(g_DataReportFlag) {
        g_DataReportFlag = 0;
        
        // 更新显示
        DisplayUpdate();
        
        // 上报数据
        printf("@%.2f,%.2f,%.2f,%.2f\r\n",
               temperature, humidity, mq2_ppm, gy302_lux);
    }
    
    // 阈值检查和报警
    ThresholdCheck();
}

/**
 * @brief 应用停止
 */
void TrashCan_OnStop(void)
{
    printf("[TrashCan] Application stopped\r\n");
    
    // 停止电机
    TB6612_SetSpeed(0, 0);
    
    // 关闭所有LED和蜂鸣器
    Beep1_TurnOff();
    Red_TurnOff();
    Green_TurnOff();
    Blue_TurnOff();
}

/**
 * @brief 应用错误处理
 */
void TrashCan_OnError(uint32_t error_code)
{
    printf("[TrashCan] Error occurred: %lu\r\n", error_code);
}

/* ========================= 私有函数实现 ========================= */

/**
 * @brief 初始化所有设备
 */
static bool InitDevices(void)
{
    bool ret = true;
    
    // OLED 初始化
    Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
    Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};
    OLED_Device_Create(&oled1, &oled_sda, &oled_scl, 0x3C);
    if(OLED_Device_Detection(&oled1) == OLED_Status_OK) {
        printf("[TrashCan] OLED detected\r\n");
        OLED_Init(&oled1);
    } else {
        printf("[TrashCan] OLED not detected\r\n");
        ret = false;
    }
    
    // GY302 初始化
    Pin_Struct gy302_sda = {GPIOC, GPIO_PIN_5};
    Pin_Struct gy302_scl = {GPIOC, GPIO_PIN_6};
    GY302_Device_Create(&gy302, &gy302_sda, &gy302_scl, 0x23);
    if(GY302_Device_Detection(&gy302) == GY302_Status_OK) {
        printf("[TrashCan] GY302 detected\r\n");
        GY302_Init(&gy302);
    } else {
        printf("[TrashCan] GY302 not detected\r\n");
        ret = false;
    }
    
    // AHT30 初始化
    Pin_Struct aht30_sda = {GPIOC, GPIO_PIN_0};
    Pin_Struct aht30_scl = {GPIOC, GPIO_PIN_13};
    AHT30_Device_Create(&aht30, &aht30_sda, &aht30_scl, 0x38);
    if(AHT30_Device_Detection(&aht30) == AHT30_Status_OK) {
        printf("[TrashCan] AHT30 detected\r\n");
        AHT30_Init(&aht30);
    } else {
        printf("[TrashCan] AHT30 not detected\r\n");
        ret = false;
    }
    
    // TB6612 初始化
    TB6612_Init();
    TB6612_SetSpeed(0, 0);
    printf("[TrashCan] TB6612 initialized\r\n");
    
    return ret;
}

/**
 * @brief 传感器数据处理
 */
static void SensorDataProcess(void)
{
    // GY302 光照数据
    GY302_Read_Lux(&gy302, &gy302_lux);
    
    // MQ-2 气体数据
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_buffer, 1);
    mq2_ppm = MQ2ConvertPPM(adc_buffer[0]);
    
    // AHT30 温湿度数据
    AHT30_Read_Data(&aht30, &temperature, &humidity);
}

/**
 * @brief 显示更新
 */
static void DisplayUpdate(void)
{
    // 显示标题
    OLED_ShowString(&oled1, 0, 0, "Gas Car", 16);
    
    // 显示MQ-2数据
    sprintf((char*)oled_buffer, "MQ2:%.1fPPM", mq2_ppm);
    OLED_ShowString(&oled1, 0, 2, (char*)oled_buffer, 16);
    
    // 显示光照数据
    sprintf((char*)oled_buffer, "Lux:%.1f", gy302_lux);
    OLED_ShowString(&oled1, 0, 4, (char*)oled_buffer, 16);
    
    // 显示温度
    sprintf((char*)oled_buffer, "Temp:%.1fC", temperature);
    OLED_ShowString(&oled1, 0, 6, (char*)oled_buffer, 16);
    
    // 显示湿度
    sprintf((char*)oled_buffer, "Hum:%.1f%%", humidity);
    OLED_ShowString(&oled1, 0, 8, (char*)oled_buffer, 16);
}

/**
 * @brief 阈值检查和报警
 */
static void ThresholdCheck(void)
{
    SensorData_t sensorData = {
        .temperature = temperature,
        .humidity = humidity,
        .co_concentration = mq2_ppm,
        .light_intensity = gy302_lux
    };
    
    AlertLevel_t alertLevel = ThresholdConfig_CheckAlert(&sensorData);
    
    if(alertLevel == ALERT_DANGER) {
        // 危险：蜂鸣器持续鸣叫，红色LED
        Beep1_TurnOn();
        Red_TurnOn();
        Green_TurnOff();
        Blue_TurnOff();
    }
    else if(alertLevel == ALERT_WARNING) {
        // 警告：蜂鸣器间歇鸣叫，红色LED
        Beep1_TurnOn();
        HAL_Delay(500);
        Beep1_TurnOff();
        Red_TurnOn();
        Green_TurnOff();
        Blue_TurnOff();
    }
    else {
        // 正常：关闭蜂鸣器，绿色LED
        Beep1_TurnOff();
        Red_TurnOff();
        Green_TurnOn();
        Blue_TurnOff();
    }
}

/**
 * @brief 电机控制
 */
static void MotorControl(void)
{
    // 方向控制
    if(direction == 1) {        // 前进
        tb6612_dir = 1;
    } else if(direction == 2) { // 后退
        tb6612_dir = -1;
    } else if(direction == 3) { // 左转
        tb6612_speed_diff = -1;
    } else if(direction == 4) { // 右转
        tb6612_speed_diff = 1;
    } else {                    // 停止
        tb6612_speed_diff = 0;
        tb6612_dir = 0;
    }
    
    // 速度控制
    switch(speed) {
        case 1: tb6612_speed = TB6612_MAX_SPEED * 0.25; break;
        case 2: tb6612_speed = TB6612_MAX_SPEED * 0.5;  break;
        case 3: tb6612_speed = TB6612_MAX_SPEED * 0.75; break;
        case 4: tb6612_speed = TB6612_MAX_SPEED;        break;
        default: tb6612_speed = 0; break;
    }
    
    // 计算左右轮速度
    int left_speed = (tb6612_speed * tb6612_dir) + (tb6612_speed * 0.7 * tb6612_speed_diff);
    int right_speed = (tb6612_speed * tb6612_dir) - (tb6612_speed * 0.7 * tb6612_speed_diff);
    
    // 设置电机速度
    TB6612_SetSpeed(left_speed, right_speed);
}

/**
 * @brief 控制指令回调函数
 */
static void OnCommandReceived(ControlCommand_t cmd)
{
    switch(cmd) {
        case CMD_STOP:
        case CMD_FORWARD:
        case CMD_BACKWARD:
        case CMD_TURN_LEFT:
        case CMD_TURN_RIGHT:
            direction = cmd;
            break;
            
        case CMD_SPEED_25:
        case CMD_SPEED_50:
        case CMD_SPEED_75:
        case CMD_SPEED_100:
            speed = cmd - 4;
            break;
            
        default:
            break;
    }
    
    // 立即控制电机
    MotorControl();
}

/* ========================= 应用入口 ========================= */

/**
 * @brief 垃圾桶应用入口函数
 */
void TrashCan_App_Main(void)
{
    // 定义应用生命周期回调
    AppLifecycleCallbacks_t callbacks = {
        .on_init = TrashCan_OnInit,
        .on_start = TrashCan_OnStart,
        .on_loop = TrashCan_OnLoop,
        .on_pause = NULL,
        .on_resume = NULL,
        .on_stop = TrashCan_OnStop,
        .on_error = TrashCan_OnError
    };
    
    // 定义应用信息
    AppInfo_t app_info = {
        .name = PROJECT_NAME,
        .version = PROJECT_VERSION,
        .description = PROJECT_DESCRIPTION,
        .type = PROJECT_TYPE,
        .state = APP_STATE_UNINIT,
        .run_time_ms = 0
    };
    
    // 初始化应用框架
    if(!AppFramework_Init(&callbacks, &app_info)) {
        printf("[TrashCan] Failed to initialize app framework\r\n");
        return;
    }
    
    // 启动应用
    if(!AppFramework_Start()) {
        printf("[TrashCan] Failed to start application\r\n");
        return;
    }
    
    // 运行应用主循环
    AppFramework_Run();
}
