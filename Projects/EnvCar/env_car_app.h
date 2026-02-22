/**
 ******************************************************************************
 * @file    env_car_app.h
 * @brief   气体泄露巡检小车应用头文件
 * @author  嵌入式开发工程师
 * @date    2026-02-20
 ******************************************************************************
 * @description
 * 基于 STM32 IoT Platform 框架开发的气体泄露巡检小车应用
 * 核心功能：
 *   1. 红外循迹巡航
 *   2. 超声波避障（动态距离设置，触发停止+声光报警，自恢复）
 *   3. 气体浓度监测报警（上下限可设，强制停机，浓度正常后恢复）
 *   4. OLED 实时循环显示（状态、浓度、距离）
 *   5. 无线通讯数据上传 + 远程指令解析（调参、人工干预）
 ******************************************************************************
 */

#ifndef __ENV_CAR_APP_H__
#define __ENV_CAR_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ========================= 巡检小车配置 ========================= */

// 气体阈值配置（默认值，可远程修改）
#define ENVCAR_GAS_THRESHOLD_HIGH       500.0f      // 甲烷浓度上限(PPM)
#define ENVCAR_GAS_THRESHOLD_LOW        10.0f       // 甲烷浓度下限(PPM)

// 避障距离配置（默认值，可远程修改）
#define ENVCAR_OBSTACLE_DISTANCE_CM     20          // 避障触发距离(cm)
#define ENVCAR_SAFE_DISTANCE_CM         30          // 安全恢复距离(cm)

// 巡航速度配置
#define ENVCAR_CRUISE_SPEED_BASE        300         // 循迹基准速度(TB6612范围 0~700)
#define ENVCAR_CRUISE_SPEED_SLOW        150         // 转弯减速速度

// 周期配置
#define ENVCAR_DATA_UPLOAD_PERIOD_MS    3000        // 无线数据上传周期(ms)
#define ENVCAR_OLED_REFRESH_PERIOD_MS   1000        // OLED页面切换周期(ms)
#define ENVCAR_SENSOR_SAMPLE_PERIOD_MS  100         // 传感器采样周期(ms)

/* ========================= 小车状态定义 ========================= */

typedef enum {
    CAR_STATE_IDLE = 0,                 // 空闲/停止
    CAR_STATE_CRUISING,                 // 循迹巡航中
    CAR_STATE_OBSTACLE_ALARM,           // 障碍物报警（已停车）
    CAR_STATE_GAS_ALARM,                // 气体报警（已强制停机）
    CAR_STATE_MANUAL,                   // 手动遥控模式
    CAR_STATE_ERROR                     // 系统错误
} CarState_t;

/* ========================= 报警类型定义 ========================= */

typedef enum {
    ALARM_NONE = 0,                     // 无报警
    ALARM_OBSTACLE,                     // 障碍物报警
    ALARM_GAS_HIGH,                     // 气体浓度过高
    ALARM_SYSTEM_ERROR                  // 系统错误
} AlarmType_t;

/* ========================= 巡检数据结构 ========================= */

/**
 * @brief 巡检小车传感器数据
 */
typedef struct {
    // 红外循迹
    bool ir_left;                       // 左路红外（true=检测到黑线）
    bool ir_center;                     // 中路红外
    bool ir_right;                      // 右路红外

    // 超声波测距
    uint16_t distance_front_cm;         // 前方距离(cm)
    uint16_t distance_side_cm;          // 侧方距离(cm)

    // 气体浓度
    float gas_ppm;                      // 气体浓度(PPM)
    uint32_t gas_adc_raw;               // MQ-2 ADC原始值

    // 环境数据
    float temperature;                  // 温度(℃)
    float humidity;                     // 湿度(%RH)

    // 数据有效性
    bool data_valid;                    // 数据有效标志
    uint32_t timestamp;                 // 时间戳(ms)
} EnvCarSensorData_t;

/**
 * @brief 控制参数（可远程动态调节）
 */
typedef struct {
    uint16_t obstacle_distance_cm;      // 避障触发距离(cm)
    uint16_t safe_distance_cm;          // 安全恢复距离(cm)
    float gas_threshold_high;           // 气体浓度上限(PPM)
    float gas_threshold_low;            // 气体浓度下限(PPM)
    int16_t cruise_speed;               // 循迹基准速度
    bool enable_gas_low_alarm;          // 使能下限报警
} EnvCarParams_t;

/**
 * @brief 小车运行状态汇总
 */
typedef struct {
    CarState_t state;                   // 当前状态
    AlarmType_t alarm;                  // 当前报警类型
    EnvCarSensorData_t sensors;         // 传感器数据
    EnvCarParams_t params;              // 控制参数
    uint32_t run_time_s;                // 运行时间(秒)
} EnvCarStatus_t;

/* ========================= 应用入口接口 ========================= */

/**
 * @brief 气体巡检小车应用主函数（入口）
 * @note  在 User_Main() 中调用, 内含无限循环, 不返回
 */
void EnvCar_App_Main(void);

/* ========================= 外部控制接口 ========================= */

/**
 * @brief 获取小车当前状态（只读）
 * @return 状态结构体常量指针
 */
const EnvCarStatus_t* EnvCar_GetStatus(void);

/**
 * @brief 远程设置控制参数
 * @param params 参数结构体指针
 * @return true: 成功
 */
bool EnvCar_SetParams(const EnvCarParams_t* params);

/**
 * @brief 手动控制小车运动
 * @param left_speed  左轮速度 (-700 ~ 700)
 * @param right_speed 右轮速度 (-700 ~ 700)
 */
void EnvCar_ManualDrive(int16_t left_speed, int16_t right_speed);

/**
 * @brief 切换到自动巡航模式
 */
void EnvCar_StartAutoCruise(void);

/**
 * @brief 停止自动巡航（进入空闲）
 */
void EnvCar_StopCruise(void);

/**
 * @brief 紧急停车
 */
void EnvCar_EmergencyStop(void);

/* ========================= AppFramework 生命周期回调 ========================= */

bool EnvCar_OnInit(void);
void EnvCar_OnStart(void);
void EnvCar_OnLoop(void);
void EnvCar_OnPause(void);
void EnvCar_OnResume(void);
void EnvCar_OnStop(void);
void EnvCar_OnError(uint32_t error_code);

#ifdef __cplusplus
}
#endif

#endif /* __ENV_CAR_APP_H__ */
