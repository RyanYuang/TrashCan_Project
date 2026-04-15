/**
 ******************************************************************************
 * @file           : EnvCar_App.h
 * @brief          : 气体泄露巡检小车应用层头文件
 * @author         : AI Assistant
 * @date           : 2026-02-20
 ******************************************************************************
 * @attention
 * 本文件实现气体泄露巡检小车的核心应用逻辑，包括：
 * 1. 红外循迹功能
 * 2. 超声波避障功能
 * 3. 气体浓度监测与报警
 * 4. OLED信息显示
 * 5. 无线通讯与远程控制
 ******************************************************************************
 */

#ifndef _ENVCAR_APP_H_
#define _ENVCAR_APP_H_

#include "main.h"
#include "Devices.h"
#include <stdbool.h>
#include <string.h>


/* ==================== 系统配置参数 ==================== */

// 系统运行状态枚举
typedef enum {
    ENVCAR_STATE_IDLE = 0,          // 空闲状态
    ENVCAR_STATE_TRACKING,          // 循迹巡航
    ENVCAR_STATE_OBSTACLE_ALARM,    // 避障报警
    ENVCAR_STATE_GAS_ALARM,         // 气体报警
    ENVCAR_STATE_MANUAL_CTRL,       // 人工控制
    ENVCAR_STATE_ERROR              // 系统错误
} EnvCar_State_Enum;

// 系统工作模式
typedef enum {
    ENVCAR_MODE_AUTO = 0,           // 自动模式（循迹+避障+监测）
    ENVCAR_MODE_MANUAL,             // 手动模式（远程控制）
    ENVCAR_MODE_STOP                // 停止模式
} EnvCar_Mode_Enum;

// 报警类型
typedef enum {
    ALARM_TYPE_NONE = 0,            // 无报警
    ALARM_TYPE_OBSTACLE,            // 障碍物报警
    ALARM_TYPE_GAS_HIGH,            // 气体浓度过高
    ALARM_TYPE_GAS_LOW,             // 气体浓度过低（可选）
    ALARM_TYPE_SYSTEM_ERROR         // 系统错误
} Alarm_Type_Enum;

/* ==================== 系统参数配置结构体 ==================== */

// 避障参数配置
typedef struct {
    uint16_t threshold_distance_cm;  // 避障距离阈值（厘米）
    uint16_t safe_distance_cm;       // 安全恢复距离（厘米）
    bool enable;                     // 避障功能使能
} ObstacleAvoidance_Config_t;

// 气体监测参数配置
typedef struct {
    float gas_threshold_high_ppm;    // 气体浓度上限（ppm）
    float gas_threshold_low_ppm;     // 气体浓度下限（ppm，可选）
    bool enable_low_alarm;           // 是否使能下限报警
    bool enable;                     // 气体监测功能使能
} GasMonitor_Config_t;

// 系统参数配置
typedef struct {
    EnvCar_Mode_Enum mode;                      // 工作模式
    int16_t tracking_speed_base;                // 循迹基准速度
    ObstacleAvoidance_Config_t obstacle_cfg;    // 避障配置
    GasMonitor_Config_t gas_cfg;                // 气体监测配置
} EnvCar_Config_t;

/* ==================== 系统运行数据结构体 ==================== */

// 传感器数据
typedef struct {
    // TODO: 红外循迹传感器数据（需要接口）
    bool ir_left;                    // 左侧红外状态（黑线检测）
    bool ir_center;                  // 中间红外状态
    bool ir_right;                   // 右侧红外状态
    
    // 超声波距离数据
    uint16_t ultrasonic1_distance_cm; /**< 超声波1距离（厘米），由 ultrasonic_task1 更新 */
    uint16_t ultrasonic2_distance_cm; /**< 超声波2；仅一路硬件时保持 0，不参与 EnvCar_MinUltrasonicCm */
    
    // 气体浓度数据
    float gas_concentration_ppm;      // 气体浓度（ppm）
    uint32_t gas_adc_value;           // 气体传感器ADC原始值
    
    // 环境数据（可选）
    float temperature;                // 温度（℃）
    float humidity;                   // 湿度（%RH）
} Sensor_Data_t;

// 系统运行状态数据
typedef struct {
    EnvCar_State_Enum current_state;  // 当前系统状态
    Alarm_Type_Enum alarm_type;       // 当前报警类型
    bool is_alarm_active;             // 报警激活标志
    uint32_t system_run_time_s;       // 系统运行时间（秒）
} System_Status_t;

/* ==================== 全局变量声明 ==================== */

extern EnvCar_Config_t g_EnvCar_Config;      // 系统配置
extern Sensor_Data_t g_Sensor_Data;          // 传感器数据
extern System_Status_t g_System_Status;      // 系统状态

/* ==================== 外部API函数声明 ==================== */

/**
 * @brief 气体巡检小车应用层初始化
 * @return 0: 成功, -1: 失败
 */
int EnvCar_App_Init(void);

/**
 * @brief 气体巡检小车主循环任务（需周期性调用）
 * @note 建议调用周期：50ms - 100ms
 */
void EnvCar_App_Task(void);

/**
 * @brief 传感器数据采集任务
 * @note 采集所有传感器数据并更新到 g_Sensor_Data
 */
void EnvCar_Sensor_Update(void);

/**
 * @brief 红外循迹控制逻辑
 * @return 0: 正常循迹, -1: 脱线
 */
int EnvCar_Tracking_Control(void);

/**
 * @brief 超声波避障检测与处理
 * @return 0: 无障碍物, 1: 检测到障碍物并处理
 */
int EnvCar_Obstacle_Detect_Handle(void);

/**
 * @brief 气体浓度监测与报警处理
 * @return 0: 浓度正常, 1: 浓度超限触发报警
 */
int EnvCar_Gas_Monitor_Handle(void);

/**
 * @brief OLED显示更新任务
 * @note 循环显示系统状态、气体浓度、超声波距离等信息
 */
void EnvCar_OLED_Display_Update(void);

/**
 * @brief 声光报警控制
 * @param alarm_type 报警类型
 * @param enable true: 启动报警, false: 停止报警
 */
void EnvCar_Alarm_Control(Alarm_Type_Enum alarm_type, bool enable);

/**
 * @brief 无线通讯数据上传任务
 * @note 通过JC278A上传传感器数据和状态信息
 */
void EnvCar_Wireless_Upload_Data(void);

/**
 * @brief 无线通讯指令解析任务
 * @note 接收并解析远程控制指令（调参、人工干预）
 */
void EnvCar_Wireless_Parse_Command(void);

/**
 * @brief 设置系统工作模式
 * @param mode 目标工作模式
 */
void EnvCar_Set_Mode(EnvCar_Mode_Enum mode);

/**
 * @brief 紧急停止
 * @note 立即停止小车运动并进入报警状态
 */
void EnvCar_Emergency_Stop(void);

/**
 * @brief 手动控制小车运动
 * @param left_speed 左轮速度 (-700 ~ 700)
 * @param right_speed 右轮速度 (-700 ~ 700)
 */
void EnvCar_Manual_Control(int16_t left_speed, int16_t right_speed);

/** USART2 接收完整行后由中断登记，主循环回显给上位机 */
void EnvCar_USART2_ScheduleHostReply_Isr(const char *frame, uint16_t len);
void EnvCar_USART2_ProcessHostReply(void);

#endif /* _ENVCAR_APP_H_ */
