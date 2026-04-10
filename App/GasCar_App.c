/**
 ******************************************************************************
 * @file           : GasCar_App.c
 * @brief          : 气体泄露巡检小车应用层实现
 * @author         : AI Assistant
 * @date           : 2026-02-20
 ******************************************************************************
 */

#include "GasCar_App.h"
#include "stdio.h"

/* ==================== 全局变量定义 ==================== */

// 系统配置参数（默认值）
EnvCar_Config_t g_EnvCar_Config = {
    .mode = ENVCAR_MODE_AUTO,
    .tracking_speed_base = 300,  // 循迹基准速度
    .obstacle_cfg = {
        .threshold_distance_cm = 20,  // 20cm触发避障
        .safe_distance_cm = 30,       // 30cm恢复运行
        .enable = true
    },
    .gas_cfg = {
        .gas_threshold_high_ppm = 500.0f,  // 500ppm触发报警
        .gas_threshold_low_ppm = 0.0f,
        .enable_low_alarm = false,
        .enable = true
    }
};

// 传感器数据
Sensor_Data_t g_Sensor_Data = {0};

// 系统状态
System_Status_t g_System_Status = {
    .current_state = ENVCAR_STATE_IDLE,
    .alarm_type = ALARM_TYPE_NONE,
    .is_alarm_active = false,
    .system_run_time_s = 0
};

/* ==================== 内部变量 ==================== */

static OLED_Handle s_oled_handle;
static JC278A_Handle s_wireless_handle;
static uint32_t s_last_tick = 0;
static uint8_t s_display_page = 0;  // OLED显示页面索引

/* ==================== 外部API实现 ==================== */

/**
 * @brief 气体巡检小车应用层初始化
 */
int EnvCar_App_Init(void)
{
    // 初始化TB6612电机驱动
    TB6612_Init();
    TB6612_Stop();
    
    // 初始化超声波传感器
    ultrasonic_init();
    
    // 初始化蜂鸣器
    Beep1_TurnOff();
    Beep2_TurnOff();
    
    // 初始化RGB指示灯
    Red_TurnOff();
    Green_TurnOff();
    Blue_TurnOff();
    
    
    // 初始化系统状态
    g_System_Status.current_state = ENVCAR_STATE_IDLE;
    g_System_Status.system_run_time_s = 0;
    s_last_tick = HAL_GetTick();
    
    return 0;
}

/**
 * @brief 气体巡检小车主循环任务
 */
void EnvCar_App_Task(void)
{
    // 更新系统运行时间
    uint32_t current_tick = HAL_GetTick();
    if (current_tick - s_last_tick >= 1000) {
        g_System_Status.system_run_time_s++;
        s_last_tick = current_tick;
    }
    
    // 1. 采集传感器数据
    EnvCar_Sensor_Update();
    
    // 2. 气体浓度监测（最高优先级）
    int gas_alarm = EnvCar_Gas_Monitor_Handle();
    if (gas_alarm == 1) {
        g_System_Status.current_state = ENVCAR_STATE_GAS_ALARM;
        g_System_Status.alarm_type = ALARM_TYPE_GAS_HIGH;
        g_System_Status.is_alarm_active = true;
        EnvCar_Emergency_Stop();
        EnvCar_Alarm_Control(ALARM_TYPE_GAS_HIGH, true);
        return;  // 气体报警时强制停机，不执行后续逻辑
    }
    
    // 3. 根据工作模式执行相应任务
    switch (g_EnvCar_Config.mode) {
        case ENVCAR_MODE_AUTO:
            // 自动模式：循迹 + 避障
            
            // 3.1 超声波避障检测
            int obstacle_detected = EnvCar_Obstacle_Detect_Handle();
            if (obstacle_detected == 1) {
                g_System_Status.current_state = ENVCAR_STATE_OBSTACLE_ALARM;
                g_System_Status.alarm_type = ALARM_TYPE_OBSTACLE;
                g_System_Status.is_alarm_active = true;
                EnvCar_Emergency_Stop();
                EnvCar_Alarm_Control(ALARM_TYPE_OBSTACLE, true);
            } else {
                // 无障碍物，执行循迹
                if (g_System_Status.is_alarm_active && 
                    g_System_Status.alarm_type == ALARM_TYPE_OBSTACLE) {
                    // 障碍物消失，恢复正常
                    EnvCar_Alarm_Control(ALARM_TYPE_OBSTACLE, false);
                    g_System_Status.is_alarm_active = false;
                }
                
                // 3.2 红外循迹控制
                int tracking_status = EnvCar_Tracking_Control();
                if (tracking_status == 0) {
                    g_System_Status.current_state = ENVCAR_STATE_TRACKING;
                } else {
                    // 脱线处理
                    TB6612_Stop();
                }
            }
            break;
            
        case ENVCAR_MODE_MANUAL:
            // 手动模式：等待远程控制指令
            g_System_Status.current_state = ENVCAR_STATE_MANUAL_CTRL;
            // 指令解析在无线通讯任务中处理
            break;
            
        case ENVCAR_MODE_STOP:
            // 停止模式
            g_System_Status.current_state = ENVCAR_STATE_IDLE;
            TB6612_Stop();
            break;
            
        default:
            break;
    }
    
    // 4. OLED显示更新
    EnvCar_OLED_Display_Update();
    
    // 5. 无线通讯数据上传
    EnvCar_Wireless_Upload_Data();
    
    // 6. 无线通讯指令解析
    EnvCar_Wireless_Parse_Command();
}

/**
 * @brief 传感器数据采集任务
 */
void EnvCar_Sensor_Update(void)
{
    // TODO: 读取红外循迹传感器状态
    // 需要接口：读取GPIO状态或专用循迹传感器接口
    // g_Sensor_Data.ir_left = IR_Read_Left();
    // g_Sensor_Data.ir_center = IR_Read_Center();
    // g_Sensor_Data.ir_right = IR_Read_Right();
    
    // 读取超声波距离
    // TODO: 缺少获取超声波距离数据的接口
    // 当前ultrasonic.h只有触发函数，没有读取距离的接口
    // g_Sensor_Data.ultrasonic1_distance_cm = Ultrasonic_Get_Distance_1();
    // g_Sensor_Data.ultrasonic2_distance_cm = Ultrasonic_Get_Distance_2();
    
    // 读取气体浓度
    // TODO: 缺少读取MQ-2 ADC数据的接口
    // g_Sensor_Data.gas_adc_value = MQ2_Get_ADC_Value();
    // g_Sensor_Data.gas_concentration_ppm = MQ2ConvertPPM(g_Sensor_Data.gas_adc_value);
    
    // 读取环境数据（可选）
    // TODO: 缺少AHT30温湿度传感器接口
    // g_Sensor_Data.temperature = AHT30_Get_Temperature();
    // g_Sensor_Data.humidity = AHT30_Get_Humidity();
}

/**
 * @brief 红外循迹控制逻辑
 */
int EnvCar_Tracking_Control(void)
{
    // TODO: 红外循迹传感器接口缺失，无法实现
    // 以下为逻辑框架（占位）
    
    /*
    int16_t left_speed = g_EnvCar_Config.tracking_speed_base;
    int16_t right_speed = g_EnvCar_Config.tracking_speed_base;
    
    // 三路循迹传感器逻辑（黑线检测）
    // 0: 白色（无黑线），1: 黑色（检测到黑线）
    
    if (g_Sensor_Data.ir_center == 1) {
        // 中间检测到黑线 -> 直行
        left_speed = g_EnvCar_Config.tracking_speed_base;
        right_speed = g_EnvCar_Config.tracking_speed_base;
    } 
    else if (g_Sensor_Data.ir_left == 1 && g_Sensor_Data.ir_center == 0) {
        // 左侧检测到黑线 -> 左转
        left_speed = g_EnvCar_Config.tracking_speed_base / 2;
        right_speed = g_EnvCar_Config.tracking_speed_base;
    } 
    else if (g_Sensor_Data.ir_right == 1 && g_Sensor_Data.ir_center == 0) {
        // 右侧检测到黑线 -> 右转
        left_speed = g_EnvCar_Config.tracking_speed_base;
        right_speed = g_EnvCar_Config.tracking_speed_base / 2;
    } 
    else if (g_Sensor_Data.ir_left == 0 && g_Sensor_Data.ir_center == 0 && g_Sensor_Data.ir_right == 0) {
        // 全部未检测到黑线 -> 脱线，停车
        TB6612_Stop();
        return -1;
    }
    
    TB6612_SetSpeed(left_speed, right_speed);
    return 0;
    */
    
    // 当前缺失接口，暂时返回成功
    return 0;
}

/**
 * @brief 超声波避障检测与处理
 */
int EnvCar_Obstacle_Detect_Handle(void)
{
    if (!g_EnvCar_Config.obstacle_cfg.enable) {
        return 0;  // 避障功能未使能
    }
    
    // TODO: 缺少读取超声波距离的接口
    // 以下为逻辑框架（占位）
    
    /*
    // 获取两个超声波传感器的最小距离
    uint16_t min_distance = g_Sensor_Data.ultrasonic1_distance_cm;
    if (g_Sensor_Data.ultrasonic2_distance_cm < min_distance) {
        min_distance = g_Sensor_Data.ultrasonic2_distance_cm;
    }
    
    // 距离判断
    if (min_distance < g_EnvCar_Config.obstacle_cfg.threshold_distance_cm) {
        // 检测到障碍物，距离过近
        return 1;
    } else if (min_distance > g_EnvCar_Config.obstacle_cfg.safe_distance_cm) {
        // 障碍物移除，距离安全
        return 0;
    } else {
        // 距离在阈值和安全距离之间，保持当前状态
        return g_System_Status.alarm_type == ALARM_TYPE_OBSTACLE ? 1 : 0;
    }
    */
    
    return 0;  // 当前缺失接口，暂时返回无障碍物
}

/**
 * @brief 气体浓度监测与报警处理
 */
int EnvCar_Gas_Monitor_Handle(void)
{
    if (!g_EnvCar_Config.gas_cfg.enable) {
        return 0;  // 气体监测功能未使能
    }
    
    // TODO: 缺少读取气体浓度的接口
    // 以下为逻辑框架（占位）
    
    /*
    float gas_ppm = g_Sensor_Data.gas_concentration_ppm;
    
    // 检查上限
    if (gas_ppm > g_EnvCar_Config.gas_cfg.gas_threshold_high_ppm) {
        return 1;  // 浓度超上限
    }
    
    // 检查下限（可选）
    if (g_EnvCar_Config.gas_cfg.enable_low_alarm) {
        if (gas_ppm < g_EnvCar_Config.gas_cfg.gas_threshold_low_ppm) {
            return 1;  // 浓度低于下限
        }
    }
    
    // 浓度恢复正常
    if (g_System_Status.alarm_type == ALARM_TYPE_GAS_HIGH) {
        // 停止报警，恢复运行
        EnvCar_Alarm_Control(ALARM_TYPE_GAS_HIGH, false);
        g_System_Status.is_alarm_active = false;
    }
    
    return 0;  // 浓度正常
    */
    
    return 0;  // 当前缺失接口，暂时返回正常
}

/**
 * @brief OLED显示更新任务
 */
void EnvCar_OLED_Display_Update(void)
{
    // TODO: 缺少完整的OLED初始化流程
    // 以下为逻辑框架（占位）
    
    /*
    static uint32_t last_update_tick = 0;
    uint32_t current_tick = HAL_GetTick();
    
    // 每1秒切换显示页面
    if (current_tick - last_update_tick >= 1000) {
        last_update_tick = current_tick;
        s_display_page = (s_display_page + 1) % 3;  // 3个页面循环
        
        OLED_Clear(&s_oled_handle);
        
        switch (s_display_page) {
            case 0:
                // 页面0: 系统状态
                OLED_ShowString(&s_oled_handle, 0, 0, "State:", 16);
                switch (g_System_Status.current_state) {
                    case ENVCAR_STATE_IDLE:
                        OLED_ShowString(&s_oled_handle, 50, 0, "IDLE", 16);
                        break;
                    case ENVCAR_STATE_TRACKING:
                        OLED_ShowString(&s_oled_handle, 50, 0, "TRACK", 16);
                        break;
                    case ENVCAR_STATE_OBSTACLE_ALARM:
                        OLED_ShowString(&s_oled_handle, 50, 0, "OBST!", 16);
                        break;
                    case ENVCAR_STATE_GAS_ALARM:
                        OLED_ShowString(&s_oled_handle, 50, 0, "GAS!", 16);
                        break;
                    case ENVCAR_STATE_MANUAL_CTRL:
                        OLED_ShowString(&s_oled_handle, 50, 0, "MANU", 16);
                        break;
                    default:
                        OLED_ShowString(&s_oled_handle, 50, 0, "ERR", 16);
                        break;
                }
                
                OLED_ShowString(&s_oled_handle, 0, 2, "Time:", 16);
                OLED_ShowNum(&s_oled_handle, 50, 2, g_System_Status.system_run_time_s, 6);
                OLED_ShowString(&s_oled_handle, 110, 2, "s", 16);
                break;
                
            case 1:
                // 页面1: 气体浓度
                OLED_ShowString(&s_oled_handle, 0, 0, "Gas:", 16);
                // TODO: 显示浮点数需要sprintf转换
                char gas_str[16];
                sprintf(gas_str, "%.1f", g_Sensor_Data.gas_concentration_ppm);
                OLED_ShowString(&s_oled_handle, 0, 2, gas_str, 16);
                OLED_ShowString(&s_oled_handle, 70, 2, "ppm", 16);
                break;
                
            case 2:
                // 页面2: 超声波距离
                OLED_ShowString(&s_oled_handle, 0, 0, "Dist1:", 16);
                OLED_ShowNum(&s_oled_handle, 60, 0, g_Sensor_Data.ultrasonic1_distance_cm, 3);
                OLED_ShowString(&s_oled_handle, 100, 0, "cm", 16);
                
                OLED_ShowString(&s_oled_handle, 0, 2, "Dist2:", 16);
                OLED_ShowNum(&s_oled_handle, 60, 2, g_Sensor_Data.ultrasonic2_distance_cm, 3);
                OLED_ShowString(&s_oled_handle, 100, 2, "cm", 16);
                break;
                
            default:
                break;
        }
    }
    */
}

/**
 * @brief 声光报警控制
 */
void EnvCar_Alarm_Control(Alarm_Type_Enum alarm_type, bool enable)
{
    if (enable) {
        // 启动报警
        switch (alarm_type) {
            case ALARM_TYPE_OBSTACLE:
                // 障碍物报警：蓝灯闪烁 + 蜂鸣器1
                Blue_Twinkle(3);
                Beep1_Alarm(3);
                break;
                
            case ALARM_TYPE_GAS_HIGH:
                // 气体报警：红灯闪烁 + 蜂鸣器2（长鸣）
                Red_TurnOn();
                Beep2_TurnOn();
                break;
                
            case ALARM_TYPE_SYSTEM_ERROR:
                // 系统错误：红蓝交替闪烁
                Red_Twinkle(5);
                Blue_Twinkle(5);
                break;
                
            default:
                break;
        }
    } else {
        // 停止报警
        Red_TurnOff();
        Green_TurnOff();
        Blue_TurnOff();
        Beep1_TurnOff();
        Beep2_TurnOff();
    }
}

/**
 * @brief 无线通讯数据上传任务
 */
void EnvCar_Wireless_Upload_Data(void)
{
    // TODO: 缺少JC278A数据发送接口的完整调用示例
    // 以下为逻辑框架（占位）
    
    /*
    static uint32_t last_upload_tick = 0;
    uint32_t current_tick = HAL_GetTick();
    
    // 每5秒上传一次数据
    if (current_tick - last_upload_tick >= 5000) {
        last_upload_tick = current_tick;
        
        // 构造上传数据包（JSON格式示例）
        char upload_buffer[256];
        sprintf(upload_buffer, 
                "{\"state\":%d,\"gas\":%.1f,\"dist1\":%d,\"dist2\":%d,\"temp\":%.1f,\"humi\":%.1f}\n",
                g_System_Status.current_state,
                g_Sensor_Data.gas_concentration_ppm,
                g_Sensor_Data.ultrasonic1_distance_cm,
                g_Sensor_Data.ultrasonic2_distance_cm,
                g_Sensor_Data.temperature,
                g_Sensor_Data.humidity);
        
        // 发送数据
        JC278A_Send_Data(&s_wireless_handle, (uint8_t*)upload_buffer, strlen(upload_buffer), 1000);
    }
    */
}

/**
 * @brief 无线通讯指令解析任务
 */
void EnvCar_Wireless_Parse_Command(void)
{
    // TODO: 缺少JC278A接收数据的完整调用示例和协议解析
    // 以下为逻辑框架（占位）
    
    /*
    uint8_t rx_buffer[128];
    uint16_t rx_length = sizeof(rx_buffer);
    
    // 非阻塞接收数据
    JC278A_Status_Enum status = JC278A_Receive_Data(&s_wireless_handle, rx_buffer, &rx_length, 10);
    
    if (status == JC278A_Status_OK && rx_length > 0) {
        // 简单的指令解析示例（实际应使用完整的协议）
        
        // 指令格式：CMD:PARAM1,PARAM2\n
        // 例如：
        // MODE:0  -> 设置自动模式
        // MODE:1  -> 设置手动模式
        // SPEED:300,300 -> 设置左右轮速度
        // DIST:25 -> 设置避障距离
        // GAS:600 -> 设置气体报警阈值
        
        if (strncmp((char*)rx_buffer, "MODE:", 5) == 0) {
            int mode = rx_buffer[5] - '0';
            EnvCar_Set_Mode((EnvCar_Mode_Enum)mode);
        }
        else if (strncmp((char*)rx_buffer, "SPEED:", 6) == 0) {
            int left_speed, right_speed;
            sscanf((char*)&rx_buffer[6], "%d,%d", &left_speed, &right_speed);
            EnvCar_Manual_Control(left_speed, right_speed);
        }
        else if (strncmp((char*)rx_buffer, "DIST:", 5) == 0) {
            int dist;
            sscanf((char*)&rx_buffer[5], "%d", &dist);
            g_EnvCar_Config.obstacle_cfg.threshold_distance_cm = dist;
        }
        else if (strncmp((char*)rx_buffer, "GAS:", 4) == 0) {
            float gas_threshold;
            sscanf((char*)&rx_buffer[4], "%f", &gas_threshold);
            g_EnvCar_Config.gas_cfg.gas_threshold_high_ppm = gas_threshold;
        }
        else if (strncmp((char*)rx_buffer, "STOP", 4) == 0) {
            EnvCar_Emergency_Stop();
        }
    }
    */
}

/**
 * @brief 设置系统工作模式
 */
void EnvCar_Set_Mode(EnvCar_Mode_Enum mode)
{
    g_EnvCar_Config.mode = mode;
    
    if (mode == ENVCAR_MODE_STOP) {
        TB6612_Stop();
    }
}

/**
 * @brief 紧急停止
 */
void EnvCar_Emergency_Stop(void)
{
    TB6612_Stop();
}

/**
 * @brief 手动控制小车运动
 */
void EnvCar_Manual_Control(int16_t left_speed, int16_t right_speed)
{
    if (g_EnvCar_Config.mode == ENVCAR_MODE_MANUAL) {
        TB6612_SetSpeed(left_speed, right_speed);
    }
}
