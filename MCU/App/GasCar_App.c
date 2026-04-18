/**
 ******************************************************************************
 * @file           : GasCar_App.c
 * @brief          : 气体泄露巡检小车应用层实现
 * @author         : AI Assistant
 * @date           : 2026-02-20
 *
 * EnvCar_App_Task 主循环步骤概览：
 *  0   USART2 回显处理 + 运行时间秒计数
 *  1   EnvCar_Sensor_Update 采集红外/超声/MQ-2
 *  1.1 可选 EnvCar_App_LogPeriodic 周期调试输出
 *  2   EnvCar_Gas_Monitor_Handle 气体去抖；报警则急停、声光、上报并 return
 *  3   气体恢复分支：关声光、清标志、按模式恢复状态字
 *  4   switch(mode)：自动=超声避障+红外循迹；手动=状态；停止=停车
 *  5~7 OLED（循迹态可开测试页：灰度位+轮速）/ 无线上传 / 无线解析（占位）
 *  8   EnvCar_StatusUplinkOnce 周期 $STS
 ******************************************************************************
 */

#include "GasCar_App.h"
#include "MQ-2.h"
#include "ultrasonic.h"
#include "line_track.h"
#include "platform_log.h"
#include "protocol_parser.h"
#include "uart_protocol.h"
#include "usart.h"
#include <stdio.h>

/** 超声波触发+捕获窗口约 21ms，节流避免主循环每圈阻塞 */
#define ENVCAR_ULTRASONIC_PERIOD_MS 10U
/** MQ-2 PPM 一阶低通系数（越大越跟手、噪声越大） */
#define ENVCAR_GAS_PPM_FILTER_ALPHA 0.25f
#define ENVCAR_GAS_TRIP_COUNT       4U
#define ENVCAR_GAS_CLEAR_COUNT      4U
/** 周期性传感器/状态 LOG 间隔（ms），依赖 PLATFORM_LOG_TEST_ENABLE */
#define ENVCAR_APP_SENSOR_LOG_PERIOD_MS 10U
/** OLED 循迹测试模式：TRACK 状态下持续显示循迹参数与速度 */
#define ENVCAR_OLED_TRACK_TEST_MODE 1U
#define ENVCAR_OLED_TRACK_REFRESH_MS 100U

/* ==================== 全局变量定义 ==================== */

// 系统配置参数（默认值）
EnvCar_Config_t g_EnvCar_Config = {
    .mode = ENVCAR_MODE_AUTO,
    .tracking_speed_base = 300,  // 循迹基准速度
    .obstacle_cfg = {
        .threshold_distance_cm = 10,  // 10cm触发避障
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

/**
 * USART2 上位机回显用的“待发送”缓存。
 * 在接收完成中断里只登记内容，由主循环 EnvCar_USART2_ProcessHostReply 再发送，
 * 避免在中断里长时间 HAL_UART_Transmit。
 */
static volatile uint8_t s_usart2_host_reply_pending;
static volatile uint16_t s_usart2_host_reply_len;
static char s_usart2_host_reply_buf[256];

/** 气体去抖：确认超限后维持的类型，供 Task 与声光使用 */
static Alarm_Type_Enum s_gas_debounced_kind = ALARM_TYPE_NONE;
static uint8_t s_gas_trip_cnt;
static uint8_t s_gas_clear_cnt;
static uint8_t s_gas_debounced_active;
static uint8_t s_log_was_gas_alarm;
static volatile uint8_t s_proto_cmd_pending;
static volatile ControlCommand_t s_proto_cmd_latest = CMD_INVALID;
static uint16_t s_manual_speed_abs = 300U;
static ControlCommand_t s_manual_motion_cmd = CMD_STOP;
static uint8_t s_track_dbg_raw4;
static int16_t s_track_dbg_left_speed;
static int16_t s_track_dbg_right_speed;
static int16_t s_track_dbg_delta;
static int16_t s_track_dbg_err100;
static uint8_t s_track_dbg_count;

static void EnvCar_ApplyManualMotion(ControlCommand_t cmd)
{
    int16_t s = (int16_t)s_manual_speed_abs;
    switch (cmd) {
        case CMD_STOP:
            TB6612_Stop();
            break;
        case CMD_FORWARD:
            TB6612_SetSpeed(s, s);
            break;
        case CMD_BACKWARD:
            TB6612_SetSpeed((int16_t)-s, (int16_t)-s);
            break;
        case CMD_TURN_LEFT:
            TB6612_SetSpeed((int16_t)-s, s);
            break;
        case CMD_TURN_RIGHT:
            TB6612_SetSpeed(s, (int16_t)-s);
            break;
        default:
            break;
    }
}

static void EnvCar_ProcessProtocolCommand(void)
{
    ControlCommand_t cmd;

    if (s_proto_cmd_pending == 0U) {
        return;
    }

    __disable_irq();
    if (s_proto_cmd_pending == 0U) {
        __enable_irq();
        return;
    }
    cmd = s_proto_cmd_latest;
    s_proto_cmd_pending = 0U;
    __enable_irq();

    /* 模式指令：直接切换模式 */
    if (cmd == CMD_MODE_MANUAL) {
        EnvCar_Set_Mode(ENVCAR_MODE_MANUAL);
        return;
    }
    if (cmd == CMD_MODE_AUTO_TRACK) {
        EnvCar_Set_Mode(ENVCAR_MODE_AUTO);
        return;
    }

    /* 速度档位：记录新速度，若当前在手动且运动指令有效则立即生效 */
    switch (cmd) {
        case CMD_SPEED_25:  s_manual_speed_abs = 175U; break;
        case CMD_SPEED_50:  s_manual_speed_abs = 300U; break;
        case CMD_SPEED_75:  s_manual_speed_abs = 500U; break;
        case CMD_SPEED_100: s_manual_speed_abs = 700U; break;
        default: break;
    }
    if (cmd >= CMD_SPEED_25 && cmd <= CMD_SPEED_100) {
        if (g_EnvCar_Config.mode == ENVCAR_MODE_MANUAL) {
            EnvCar_ApplyManualMotion(s_manual_motion_cmd);
        }
        return;
    }

    /* 运动指令：切到手动并执行 */
    if (cmd <= CMD_TURN_RIGHT) {
        if (g_EnvCar_Config.mode != ENVCAR_MODE_MANUAL) {
            EnvCar_Set_Mode(ENVCAR_MODE_MANUAL);
        }
        s_manual_motion_cmd = cmd;
        EnvCar_ApplyManualMotion(cmd);
    }
}

#if PLATFORM_LOG_ENABLE && PLATFORM_LOG_TEST_ENABLE
/** 周期性打印：传感器 + 状态（避免刷屏） */
static void EnvCar_App_LogPeriodic(void)
{
    static uint32_t s_log_tick;
    uint32_t now = HAL_GetTick();
    /* 步骤1：未到打印周期则直接返回 */
    if ((now - s_log_tick) < ENVCAR_APP_SENSOR_LOG_PERIOD_MS) {
        return;
    }
    /* 步骤2：刷新周期起点 */
    s_log_tick = now;

    /* 步骤3：输出一行综合状态（模式/状态/报警/传感器） */
    LOG_TEST("[EnvCar] mode=%u state=%u alarm=%u act=%u adc=%lu ppm=%.1f US1=%u US2=%u IR=%u%u%u raw=0x%02X\r\n",
             (unsigned)g_EnvCar_Config.mode,
             (unsigned)g_System_Status.current_state,
             (unsigned)g_System_Status.alarm_type,
             (unsigned)g_System_Status.is_alarm_active,
             (unsigned long)g_Sensor_Data.gas_adc_value,
             (double)g_Sensor_Data.gas_concentration_ppm,
             (unsigned)g_Sensor_Data.ultrasonic1_distance_cm,
             (unsigned)g_Sensor_Data.ultrasonic2_distance_cm,
             (unsigned)g_Sensor_Data.ir_left,
             (unsigned)g_Sensor_Data.ir_center,
             (unsigned)g_Sensor_Data.ir_right,
             (unsigned)LineTrack_ReadRaw4());
}
#endif

/** 有效超声波距离 (cm)：仅一路硬件时等价于路1；路2 非 0 时取两路较小值 */
static uint16_t EnvCar_MinUltrasonicCm(void)
{
    /* 步骤1：读取缓存（单路工程下路2 恒为 0，min 即路1） */
    uint16_t d1 = g_Sensor_Data.ultrasonic1_distance_cm;
    uint16_t d2 = g_Sensor_Data.ultrasonic2_distance_cm;
    uint16_t m = 0;
    /* 步骤2：先取路1有效值（非0 表示最近一次测到有效回波） */
    if (d1 > 0U) {
        m = d1;
    }
    /* 步骤3：路2有效且更小时，改用路2 */
    if (d2 > 0U && (m == 0U || d2 < m)) {
        m = d2;
    }
    return m;
}

/** 周期上报 USART2 状态帧 $STS:...（内部 300ms 节流） */
static void EnvCar_StatusUplinkOnce(void)
{
    static uint32_t s_last_uplink_ms;
    uint32_t now = HAL_GetTick();
    /* 步骤1：300ms 内已发送过则跳过，减轻串口负载 */
    if ((now - s_last_uplink_ms) < 10U) {
        return;
    }
    /* 步骤2：记录本次发送时刻 */
    s_last_uplink_ms = now;

    /* 步骤3：取有效最小距离，计算障碍侧风险与上报距离 */
    uint16_t min_cm = EnvCar_MinUltrasonicCm();
    uint8_t obs_flag = UART_OBS_NONE;
    uint16_t obs_cm = 0;
    if (g_EnvCar_Config.obstacle_cfg.enable && min_cm > 0U &&
        min_cm < g_EnvCar_Config.obstacle_cfg.threshold_distance_cm) {
        obs_flag = UART_OBS_NEAR;
        obs_cm = min_cm;
    }

    /* 步骤4：按阈值判断气体风险位（与配置 enable / 高低限一致） */
    float ppm = g_Sensor_Data.gas_concentration_ppm;
    uint8_t gas_risk = 0U;
    if (g_EnvCar_Config.gas_cfg.enable) {
        if (ppm > g_EnvCar_Config.gas_cfg.gas_threshold_high_ppm) {
            gas_risk = 1U;
        } else if (g_EnvCar_Config.gas_cfg.enable_low_alarm &&
                   ppm < g_EnvCar_Config.gas_cfg.gas_threshold_low_ppm) {
            gas_risk = 1U;
        }
    }

    /* 步骤5：组合障碍与气体，得到协议用 alarm 编码 */
    uint8_t obs_risk = (obs_flag == UART_OBS_NEAR) ? 1U : 0U;
    uint8_t alarm;
    if (obs_risk != 0U && gas_risk != 0U) {
        alarm = UART_ALM_BOTH;
    } else if (gas_risk != 0U) {
        alarm = UART_ALM_GAS;
    } else if (obs_risk != 0U) {
        alarm = UART_ALM_OBST;
    } else {
        alarm = UART_ALM_NONE;
    }

    /* 步骤6：组帧并通过 USART2 发出 */
    UART_StatusUplink_t st = {
        .gas_ppm = ppm,
        .obs_flag = obs_flag,
        .obs_cm = obs_cm,
        .alarm = alarm,
        .car_state = (uint8_t)g_System_Status.current_state,
        .run_time_s = g_System_Status.system_run_time_s,
    };
    (void)UART_Protocol_SendStatusFrame(&huart2, &st);
}

/** 串口 # 阈值帧解析成功后写入 g_EnvCar_Config */
static void EnvCar_OnProtocolThreshold(const ProtocolEnvLimits_t *lim)
{
    /* 步骤1：空指针保护 */
    if (lim == NULL) {
        return;
    }
    /* 步骤2：写入避障触发距离与安全恢复距离 */
    g_EnvCar_Config.obstacle_cfg.threshold_distance_cm = lim->obstacle_trig_cm;
    g_EnvCar_Config.obstacle_cfg.safe_distance_cm = lim->obstacle_safe_cm;
    /* 步骤3：写入气体高低限；low>0 时使能下限报警 */
    g_EnvCar_Config.gas_cfg.gas_threshold_low_ppm = lim->gas_low_ppm;
    g_EnvCar_Config.gas_cfg.gas_threshold_high_ppm = lim->gas_high_ppm;
    g_EnvCar_Config.gas_cfg.enable_low_alarm = (lim->gas_low_ppm > 0.0f);
}

/**
 * @brief 在中断里登记一帧待回显到上位机（USART2）的文本。
 *
 * 典型调用链：HAL_UART_RxCpltCallback → 本函数。仅 memcpy 并置 pending，
 * 不发送串口，以缩短 ISR 执行时间并降低与 HAL 收发状态冲突的风险。
 *
 * @param frame 已解析的一行协议文本（不含行尾回车/换行）。
 * @param len   有效字节数；过长时截断，并保证缓冲末尾为字符串结束符。
 */
void EnvCar_USART2_ScheduleHostReply_Isr(const char *frame, uint16_t len)
{
    /* 步骤1：参数合法性 */
    if (frame == NULL || len == 0) {
        return;
    }
    /* 步骤2：超长截断，保留末尾 '\0' 空间 */
    if (len >= sizeof(s_usart2_host_reply_buf)) {
        len = (uint16_t)(sizeof(s_usart2_host_reply_buf) - 1U);
    }
    /* 步骤3：拷贝到共享缓冲并置 pending，由主循环发送 */
    memcpy(s_usart2_host_reply_buf, frame, len);
    s_usart2_host_reply_buf[len] = '\0';
    s_usart2_host_reply_len = len;
    s_usart2_host_reply_pending = 1;
}

/**
 * @brief 在主循环中处理 USART2 上位机回显：取出待发送内容并发出。
 *
 * 应在 EnvCar_App_Task（或等价主循环）中周期性调用。若 pending 为真，
 * 短暂关中断把共享缓冲拷到栈上、清除 pending，再开中断后格式化为
 * ECHO:原文 + 回车换行，经 USART2 阻塞发送；LOG_TEST 用于另一调试后端输出同内容。
 */
void EnvCar_USART2_ProcessHostReply(void)
{
    /* 步骤1：无待发送内容则返回 */
    if (!s_usart2_host_reply_pending) {
        return;
    }

    char local[256];
    uint16_t len;

    /* 步骤2：关中断，双检 pending，把 ISR 写入的缓冲搬到栈上并清 pending */
    __disable_irq();
    if (!s_usart2_host_reply_pending) {
        __enable_irq();
        return;
    }
    len = s_usart2_host_reply_len;
    if (len >= sizeof(local)) {
        len = (uint16_t)(sizeof(local) - 1U);
    }
    memcpy(local, s_usart2_host_reply_buf, len);
    local[len] = '\0';
    s_usart2_host_reply_pending = 0;
    __enable_irq();

    /* 步骤3：组 ECHO 行并阻塞发 USART2，同时可选 LOG_TEST */
    char line[288];
    int n = snprintf(line, sizeof(line), "ECHO:%s\r\n", local);
    if (n > 0 && n < (int)sizeof(line)) {
        (void)HAL_UART_Transmit(&huart2, (uint8_t *)line, (uint16_t)n, 200);
        LOG_TEST("%s", line);
    }
    
}

static int EnvCar_OLED_Init(void)
{
    OLED_Status_Enum oled_status;
    Pin_Struct oled_sda = {GPIOB, GPIO_PIN_14};
    Pin_Struct oled_scl = {GPIOB, GPIO_PIN_15};

    /* 步骤1：创建 I2C 软件总线设备句柄 */
    oled_status = OLED_Device_Create(&s_oled_handle, &oled_sda, &oled_scl, 0x3C);
    if (oled_status != OLED_Status_OK) {
        return -1;
    }

    /* 步骤2：探测 OLED 是否应答 */
    oled_status = OLED_Device_Detection(&s_oled_handle);
    if (oled_status != OLED_Status_OK) {
        return -1;
    }

    /* 步骤3：发送初始化命令序列 */
    oled_status = OLED_Init(&s_oled_handle);
    if (oled_status != OLED_Status_OK) {
        return -1;
    }

    /* 步骤4：清屏并显示开机提示 */
    OLED_Clear(&s_oled_handle);
    OLED_ShowString(&s_oled_handle, 0, 0, "EnvCar Init", 16);
    return 0;
}

/**
 * @brief USART2 @ 指令回调：将模式类指令映射到 g_EnvCar_Config.mode。
 * @note 在 UART 接收完成中断里触发；仅做轻量状态切换，避免长耗时。
 */
static void EnvCar_OnProtocolCommand(ControlCommand_t cmd)
{
    /* 中断上下文仅登记命令，避免在 ISR 执行电机/耗时逻辑 */
    s_proto_cmd_latest = cmd;
    s_proto_cmd_pending = 1U;
}

/* ==================== 外部API实现 ==================== */

/**
 * @brief 气体巡检小车应用层初始化
 */
int EnvCar_App_Init(void)
{
    /* 步骤1：电机驱动初始化并默认停车 */
    TB6612_Init();
    TB6612_Stop();

    /* 步骤2：超声波 TIM 输入捕获与触发脚 */
    ultrasonic_init();

    /* 步骤3：蜂鸣器默认静音 */
    Beep1_TurnOff();

    /* 步骤4：RGB 灯默认全灭 */
    Red_TurnOff();
    Green_TurnOff();
    Blue_TurnOff();

    /* 步骤5：OLED 显示设备 */
    if (EnvCar_OLED_Init() != 0) {
        return -1;
    }

    /* 步骤6：应用层状态与运行时间基准 */
    g_System_Status.current_state = ENVCAR_STATE_IDLE;
    g_System_Status.system_run_time_s = 0;
    s_last_tick = HAL_GetTick();

    /* 步骤7：串口协议解析与回调注册 */
    Protocol_Parser_Init();
    Protocol_Parser_RegisterThresholdCallback(EnvCar_OnProtocolThreshold);
    Protocol_Parser_RegisterCommandCallback(EnvCar_OnProtocolCommand);
    /* 步骤8：USART2 协议栈（与上位机通信） */
    if (UART_Protocol_Init(&huart2) != HAL_OK) {
        return -1;
    }

    /* 步骤9：调试日志提示初始化完成 */
    LOG_TEST("[EnvCar] App_Init OK (log period %ums)\r\n", (unsigned)ENVCAR_APP_SENSOR_LOG_PERIOD_MS);

    return 0;
}

/**
 * @brief 气体巡检小车主循环任务
 * @note 建议周期 50~100ms 调用。流程顺序：串口回显 → 运行时间 → 采集 → 日志 → 气体 → 模式任务 → OLED/无线 → 状态上报。
 */
void EnvCar_App_Task(void)
{
    // 循环次数LOG_TEST
    static uint32_t s_loop_count = 0;
    s_loop_count++;
    LOG_TEST("[EnvCar] loop_count=%u\r\n", s_loop_count);
    /* 步骤0：处理 USART2 上位机回显（中断登记、主循环发送） */
    EnvCar_USART2_ProcessHostReply();
    /* 步骤0.2：处理串口协议命令（中断登记，主循环执行） */
    EnvCar_ProcessProtocolCommand();

    /* 步骤0.1：每秒递增系统运行时间 */
    uint32_t current_tick = HAL_GetTick();
    if (current_tick - s_last_tick >= 1000) {
        g_System_Status.system_run_time_s++;
        s_last_tick = current_tick;
    }

    /* 步骤1：采集全部传感器数据写入 g_Sensor_Data */
    EnvCar_Sensor_Update();

#if PLATFORM_LOG_ENABLE && PLATFORM_LOG_TEST_ENABLE
    /* 步骤1.1：周期性调试打印（受宏与周期控制） */
    EnvCar_App_LogPeriodic();
#endif

    /* 步骤2：气体监测（去抖后）；超限则最高优先级停机+报警并结束本周期 */
    int gas_alarm = EnvCar_Gas_Monitor_Handle();
    if (gas_alarm == 1) {
        /* 步骤2a：首次进入气体报警时打一条边沿日志 */
        if (s_log_was_gas_alarm == 0U) {
            LOG_TEST("[EnvCar] GAS TRIP kind=%u ppm=%.1f thr_high=%.1f thr_low=%.1f low_en=%u\r\n",
                     (unsigned)s_gas_debounced_kind,
                     (double)g_Sensor_Data.gas_concentration_ppm,
                     (double)g_EnvCar_Config.gas_cfg.gas_threshold_high_ppm,
                     (double)g_EnvCar_Config.gas_cfg.gas_threshold_low_ppm,
                     (unsigned)g_EnvCar_Config.gas_cfg.enable_low_alarm);
        }
        s_log_was_gas_alarm = 1U;
        /* 步骤2b：置状态、急停、声光报警、立即上报一帧状态 */
        g_System_Status.current_state = ENVCAR_STATE_GAS_ALARM;
        g_System_Status.alarm_type = s_gas_debounced_kind;
        g_System_Status.is_alarm_active = true;
        EnvCar_Emergency_Stop();
        EnvCar_Alarm_Control(g_System_Status.alarm_type, true);
        EnvCar_StatusUplinkOnce();
        return;  /* 本周期不再执行循迹/避障等后续逻辑 */
    }

    /* 步骤3：气体已恢复安全 → 关声光、清报警标志并按模式恢复业务状态 */
    if (g_System_Status.current_state == ENVCAR_STATE_GAS_ALARM ||
        g_System_Status.alarm_type == ALARM_TYPE_GAS_HIGH ||
        g_System_Status.alarm_type == ALARM_TYPE_GAS_LOW) {
        if (s_log_was_gas_alarm != 0U) {
            LOG_TEST("[EnvCar] GAS cleared mode=%u ppm=%.1f\r\n",
                     (unsigned)g_EnvCar_Config.mode,
                     (double)g_Sensor_Data.gas_concentration_ppm);
        }
        s_log_was_gas_alarm = 0U;
        EnvCar_Alarm_Control(ALARM_TYPE_GAS_HIGH, false);
        g_System_Status.is_alarm_active = false;
        g_System_Status.alarm_type = ALARM_TYPE_NONE;
        if (g_EnvCar_Config.mode == ENVCAR_MODE_AUTO) {
            g_System_Status.current_state = ENVCAR_STATE_TRACKING;
        } else if (g_EnvCar_Config.mode == ENVCAR_MODE_MANUAL) {
            g_System_Status.current_state = ENVCAR_STATE_MANUAL_CTRL;
        } else {
            g_System_Status.current_state = ENVCAR_STATE_IDLE;
        }
    }

    /* 步骤4：按当前工作模式执行（自动=避障+循迹；手动=待机控车；停止=空闲+停车） */
    switch (g_EnvCar_Config.mode) {
        case ENVCAR_MODE_AUTO: {
            static uint8_t s_log_was_obstacle;

            /* 步骤4.1：超声波避障判定（滞回） */
            int obstacle_detected = EnvCar_Obstacle_Detect_Handle();
            if (obstacle_detected == 1) {
                if (s_log_was_obstacle == 0U) {
                    LOG_TEST("[EnvCar] OBSTACLE min_cm=%u th=%u safe=%u\r\n",
                             (unsigned)EnvCar_MinUltrasonicCm(),
                             (unsigned)g_EnvCar_Config.obstacle_cfg.threshold_distance_cm,
                             (unsigned)g_EnvCar_Config.obstacle_cfg.safe_distance_cm);
                }
                s_log_was_obstacle = 1U;
                /* 步骤4.1a：有障碍 → 停车、声光、置障碍报警状态 */
                g_System_Status.current_state = ENVCAR_STATE_OBSTACLE_ALARM;
                g_System_Status.alarm_type = ALARM_TYPE_OBSTACLE;
                g_System_Status.is_alarm_active = true;
                EnvCar_Emergency_Stop();
                EnvCar_Alarm_Control(ALARM_TYPE_OBSTACLE, true);
            } else {
                /* 步骤4.2：无障碍时，若此前为障碍报警则解除声光 */
                if (g_System_Status.is_alarm_active &&
                    g_System_Status.alarm_type == ALARM_TYPE_OBSTACLE) {
                    if (s_log_was_obstacle != 0U) {
                        LOG_TEST("[EnvCar] OBSTACLE cleared min_cm=%u\r\n",
                                 (unsigned)EnvCar_MinUltrasonicCm());
                    }
                    s_log_was_obstacle = 0U;
                    EnvCar_Alarm_Control(ALARM_TYPE_OBSTACLE, false);
                    g_System_Status.is_alarm_active = false;
                }

                /* 步骤4.3：红外循迹；脱线则停轮 */
                int tracking_status = EnvCar_Tracking_Control();
                if (tracking_status == 0) {
                    g_System_Status.current_state = ENVCAR_STATE_TRACKING;
                } else {
                    TB6612_Stop();
                }
            }
            break;
        }

        case ENVCAR_MODE_MANUAL:
            /* 步骤4M：手动模式仅更新状态，速度由无线/串口指令设置 */
            g_System_Status.current_state = ENVCAR_STATE_MANUAL_CTRL;
            break;

        case ENVCAR_MODE_STOP:
            /* 步骤4S：停止模式 → 空闲 + 电机停 */
            g_System_Status.current_state = ENVCAR_STATE_IDLE;
            TB6612_Stop();
            break;

        default:
            break;
    }

    /* 步骤4.9：正常状态指示灯——无报警时常亮绿灯 */
    if (!g_System_Status.is_alarm_active &&
        g_System_Status.alarm_type == ALARM_TYPE_NONE) {
        Red_TurnOff();
        Blue_TurnOff();
        Green_TurnOn();
    }

    /* 步骤5：OLED 内容刷新（当前多为占位） */
    EnvCar_OLED_Display_Update();

    /* 步骤6：无线模块数据上传（占位） */
    EnvCar_Wireless_Upload_Data();

    /* 步骤7：无线模块指令解析（占位） */
    EnvCar_Wireless_Parse_Command();

    /* 步骤8：USART2 周期上报 $STS（内部 300ms 节流） */
    EnvCar_StatusUplinkOnce();
}

/**
 * @brief 传感器数据采集任务
 * @note 顺序：五路红外→三路逻辑 → 超声波（节流）→ MQ-2 ADC/PPM 滤波。
 */
void EnvCar_Sensor_Update(void)
{
    /* 步骤1：四路红外采样并合成左/中/右三路循迹逻辑量 */
    LineTrack_ApplyToLogic(&g_Sensor_Data.ir_left, &g_Sensor_Data.ir_center, &g_Sensor_Data.ir_right);

    /* 步骤2：仅一路超声波 — 周期调用 ultrasonic_task1(Trig1) 并读路1距离；路2 固定为 0 */
    {
        static uint32_t s_us_last_ms;
        uint32_t now = HAL_GetTick();
        if ((now - s_us_last_ms) >= ENVCAR_ULTRASONIC_PERIOD_MS) {
            s_us_last_ms = now;
            ultrasonic_task1();
            g_Sensor_Data.ultrasonic1_distance_cm = Ultrasonic_Get_Distance_Cm_1();
            g_Sensor_Data.ultrasonic2_distance_cm = 0U;
        }
    }

    /* 步骤3：MQ-2 — ADC 原始值、换算 PPM、一阶低通后写入全局浓度供报警与上报 */
    {
        uint32_t raw = MQ2_ReadRaw();
        g_Sensor_Data.gas_adc_value = raw;
        float ppm_raw = MQ2ConvertPPM(raw);
        static uint8_t s_gas_filt_inited;
        static float s_gas_ppm_filt;
        if (s_gas_filt_inited == 0U) {
            s_gas_ppm_filt = ppm_raw;
            s_gas_filt_inited = 1U;
        } else {
            s_gas_ppm_filt = (ENVCAR_GAS_PPM_FILTER_ALPHA * ppm_raw) +
                             ((1.0f - ENVCAR_GAS_PPM_FILTER_ALPHA) * s_gas_ppm_filt);
        }
        g_Sensor_Data.gas_concentration_ppm = s_gas_ppm_filt;
    }

    /* 步骤4（可选）：温湿度等环境量，接入驱动后在此更新 */
    // TODO: 缺少AHT30温湿度传感器接口
    // g_Sensor_Data.temperature = AHT30_Get_Temperature();
    // g_Sensor_Data.humidity = AHT30_Get_Humidity();
}

/**
 * @brief 红外循迹控制逻辑（四路车体坐标：外左、左中、右中、外右，由 LineTrack_ReadSpatial4 映射 Gay4..Gay1）
 * @note 高电平=在黑线上。外侧重判大幅纠偏；中间仅一侧见线小幅纠偏；双中均在且无外侧时压低误差。
 * @return 0 正常循迹；-1 四路均未见线（脱线）
 */
int EnvCar_Tracking_Control(void)
{
    int16_t base = g_EnvCar_Config.tracking_speed_base;
    int16_t left_speed = base;
    int16_t right_speed = base;
    /* bit0 外左(Gay4) … bit3 外右(Gay1)，与车头从左到右一致 */
    uint8_t raw = LineTrack_ReadSpatial4();

    bool g1 = (raw & (1U << 0)) != 0U;
    bool g2 = (raw & (1U << 1)) != 0U;
    bool g3 = (raw & (1U << 2)) != 0U;
    bool g4 = (raw & (1U << 3)) != 0U;

    uint8_t n = (uint8_t)((g1 ? 1U : 0U) + (g2 ? 1U : 0U) + (g3 ? 1U : 0U) + (g4 ? 1U : 0U));

    if (n == 0U) {
        s_track_dbg_raw4 = raw;
        s_track_dbg_err100 = 0;
        s_track_dbg_count = 0;
        s_track_dbg_delta = 0;
        s_track_dbg_left_speed = 0;
        s_track_dbg_right_speed = 0;
        TB6612_Stop();
        return -1;
    }

    /*
     * 误差方向与原先五路加权一致：左侧见线贡献为正 → e>0 → delta>0 → left=base+delta 更快。
     * （与旧版 Gay1..+、Gay5..- 同一套差速习惯，避免电机接线侧反了。）
     */
    int32_t e100 = 0;

    /* 外侧见线：大幅纠偏（左右对称；系数为原先外侧权重的 2 倍） */
    if (g1) {
        e100 += 1000;
    }
    if (g4) {
        e100 -= 1000;
    }
    if (g1 && g4) {
        e100 = 0;
    }

    /* 中间两路：仅一侧见线时小幅纠偏 */
    if (g2 != g3) {
        if (g2) {
            e100 += 45;
        }
        if (g3) {
            e100 -= 45;
        }
    }

    /* 双中均在黑线上、且外侧都未见线：认为对中较好，减弱残余误差减少蛇形 */
    if (g2 && g3 && !g1 && !g4) {
        e100 /= 4;
    }

    float e = (float)e100 / 100.0f;
    float k = 1.0f;
    int16_t delta = (int16_t)(k * e * (float)base);

    /* 差速上限：原为 0.9*base，放宽为与基准同量级（100% base） */
    int16_t max_d = base;
    if (delta > max_d) {
        delta = max_d;
    }
    if (delta < (int16_t)-max_d) {
        delta = (int16_t)-max_d;
    }

    left_speed = (int16_t)(base + delta);
    right_speed = (int16_t)(base - delta);

    s_track_dbg_raw4 = raw;
    s_track_dbg_err100 = (int16_t)e100;
    s_track_dbg_count = n;
    s_track_dbg_delta = delta;
    s_track_dbg_left_speed = left_speed;
    s_track_dbg_right_speed = right_speed;

    TB6612_SetSpeed((int)left_speed, (int)right_speed);
    return 0;
}

/**
 * @brief 超声波避障检测与处理
 * @return 1 判定应避障停车；0 可继续行驶
 */
int EnvCar_Obstacle_Detect_Handle(void)
{
    /* 步骤1：功能关闭则始终视为无障碍 */
    if (!g_EnvCar_Config.obstacle_cfg.enable) {
        return 0;
    }

    /* 步骤2：取有效最近距离（单路时即 US1）；为 0 表示暂无测距结果，不报障 */
    uint16_t min_cm = EnvCar_MinUltrasonicCm();
    if (min_cm == 0U) {
        return 0;
    }

    uint16_t th = g_EnvCar_Config.obstacle_cfg.threshold_distance_cm;
    uint16_t safe = g_EnvCar_Config.obstacle_cfg.safe_distance_cm;

    /* 步骤3：小于触发阈值 → 报障 */
    if (min_cm < th) {
        return 1;
    }
    /* 步骤4：大于安全距离 → 解除 */
    if (min_cm > safe) {
        return 0;
    }
    /* 步骤5：滞回区 — 已处于障碍报警则保持 1，否则 0，避免边界抖动 */
    return (g_System_Status.alarm_type == ALARM_TYPE_OBSTACLE) ? 1 : 0;
}

/**
 * @brief 气体浓度监测与报警处理（连续采样去抖）
 * @return 1 当前应视为气体报警态；0 安全
 */
int EnvCar_Gas_Monitor_Handle(void)
{
    /* 步骤1：气体监测关闭时复位内部去抖状态并返回安全 */
    if (!g_EnvCar_Config.gas_cfg.enable) {
        s_gas_trip_cnt = 0;
        s_gas_clear_cnt = 0;
        s_gas_debounced_active = 0U;
        s_gas_debounced_kind = ALARM_TYPE_NONE;
        return 0;
    }

    /* 步骤2：用滤波后 PPM 与配置比较，得到瞬时越上限/下限 */
    float ppm = g_Sensor_Data.gas_concentration_ppm;
    const GasMonitor_Config_t *cfg = &g_EnvCar_Config.gas_cfg;
    int instant_high = (ppm > cfg->gas_threshold_high_ppm) ? 1 : 0;
    int instant_low = (cfg->enable_low_alarm && ppm < cfg->gas_threshold_low_ppm) ? 1 : 0;
    int instant = (instant_high || instant_low) ? 1 : 0;

    /* 步骤3：瞬时异常则累加 trip 计数、清零 clear；瞬时正常则相反 */
    if (instant != 0) {
        s_gas_clear_cnt = 0U;
        if (s_gas_trip_cnt < 255U) {
            s_gas_trip_cnt++;
        }
    } else {
        s_gas_trip_cnt = 0U;
        if (s_gas_clear_cnt < 255U) {
            s_gas_clear_cnt++;
        }
    }

    /* 步骤4：已处于去抖报警态时，连续 M 次正常才解除 */
    if (s_gas_debounced_active != 0U) {
        if (s_gas_clear_cnt >= ENVCAR_GAS_CLEAR_COUNT) {
            s_gas_debounced_active = 0U;
            s_gas_debounced_kind = ALARM_TYPE_NONE;
        }
    } else {
        /* 步骤5：未报警时，连续 N 次瞬时异常才确认报警并记录高/低类型 */
        if (s_gas_trip_cnt >= ENVCAR_GAS_TRIP_COUNT) {
            s_gas_debounced_active = 1U;
            s_gas_debounced_kind = (instant_high != 0) ? ALARM_TYPE_GAS_HIGH : ALARM_TYPE_GAS_LOW;
        }
    }

    return (s_gas_debounced_active != 0U) ? 1 : 0;
}

/**
 * @brief OLED显示更新任务
 * @note 默认：状态 + 甲烷 + 障碍物。循迹测试模式见 ENVCAR_OLED_TRACK_TEST_MODE。
 */
void EnvCar_OLED_Display_Update(void)
{
    static uint32_t s_last_update_tick;
    uint32_t current_tick = HAL_GetTick();
    char line[24];
    const char *state_text = "ERR";
    uint16_t obs_cm = EnvCar_MinUltrasonicCm();
    const char *obs_text;

#if ENVCAR_OLED_TRACK_TEST_MODE
    /* 循迹态：快刷显示灰度（空间序 4 位 + 硬件半字节）与左右轮设定速度 */
    if (g_System_Status.current_state == ENVCAR_STATE_TRACKING) {
        if ((current_tick - s_last_update_tick) < ENVCAR_OLED_TRACK_REFRESH_MS) {
            return;
        }
        s_last_update_tick = current_tick;

        uint8_t sp = LineTrack_ReadSpatial4();
        uint8_t hw = LineTrack_ReadRaw4();
        char s_bits[5];
        s_bits[0] = (char)('0' + ((sp >> 0) & 1U));
        s_bits[1] = (char)('0' + ((sp >> 1) & 1U));
        s_bits[2] = (char)('0' + ((sp >> 2) & 1U));
        s_bits[3] = (char)('0' + ((sp >> 3) & 1U));
        s_bits[4] = '\0';

        (void)OLED_Clear(&s_oled_handle);
        (void)snprintf(line, sizeof(line), "S:%s H:%X n:%u", s_bits, (unsigned)(hw & 0x0FU),
                       (unsigned)s_track_dbg_count);
        (void)OLED_ShowString(&s_oled_handle, 0, 0, line, 16);
        {
            int ls = (int)s_track_dbg_left_speed;
            int rs = (int)s_track_dbg_right_speed;
            if (ls > 999) {
                ls = 999;
            }
            if (ls < -999) {
                ls = -999;
            }
            if (rs > 999) {
                rs = 999;
            }
            if (rs < -999) {
                rs = -999;
            }
            (void)snprintf(line, sizeof(line), "L:%4d R:%4d", ls, rs);
        }
        (void)OLED_ShowString(&s_oled_handle, 0, 2, line, 16);
        {
            int de = (int)s_track_dbg_err100;
            if (de > 9999) {
                de = 9999;
            }
            if (de < -9999) {
                de = -9999;
            }
            (void)snprintf(line, sizeof(line), "d:%4d e:%5d", (int)s_track_dbg_delta, de);
        }
        (void)OLED_ShowString(&s_oled_handle, 0, 4, line, 16);
        if (obs_cm == 0U) {
            obs_text = "N/A";
        } else if (g_EnvCar_Config.obstacle_cfg.enable &&
                   obs_cm < g_EnvCar_Config.obstacle_cfg.threshold_distance_cm) {
            obs_text = "NEAR";
        } else {
            obs_text = "OK";
        }
        (void)snprintf(line, sizeof(line), "Ob:%ucm %s", (unsigned)obs_cm, obs_text);
        (void)OLED_ShowString(&s_oled_handle, 0, 6, line, 16);
        return;
    }
#endif

    /* 步骤1：1 秒节流刷新，避免频繁刷屏 */
    if ((current_tick - s_last_update_tick) < 1000U) {
        return;
    }
    s_last_update_tick = current_tick;

    /* 步骤2：状态字符串映射 */
    switch (g_System_Status.current_state) {
        case ENVCAR_STATE_IDLE:           state_text = "IDLE";  break;
        case ENVCAR_STATE_TRACKING:       state_text = "TRACK"; break;
        case ENVCAR_STATE_OBSTACLE_ALARM: state_text = "OBST";  break;
        case ENVCAR_STATE_GAS_ALARM:      state_text = "GAS";   break;
        case ENVCAR_STATE_MANUAL_CTRL:    state_text = "MANU";  break;
        default:                          state_text = "ERR";   break;
    }

    /* 步骤3：障碍物距离文本（无回波/近障/安全） */
    if (obs_cm == 0U) {
        obs_text = "N/A";
    } else if (g_EnvCar_Config.obstacle_cfg.enable &&
               obs_cm < g_EnvCar_Config.obstacle_cfg.threshold_distance_cm) {
        obs_text = "NEAR";
    } else {
        obs_text = "SAFE";
    }

    /* 步骤4：单页显示三类关键信息 */
    (void)OLED_Clear(&s_oled_handle);
    (void)snprintf(line, sizeof(line), "State:%s", state_text);
    (void)OLED_ShowString(&s_oled_handle, 0, 0, line, 16);
    (void)snprintf(line, sizeof(line), "CH4:%.1fppm", (double)g_Sensor_Data.gas_concentration_ppm);
    (void)OLED_ShowString(&s_oled_handle, 0, 2, line, 16);
    (void)snprintf(line, sizeof(line), "Obs:%ucm %s", (unsigned)obs_cm, obs_text);
    (void)OLED_ShowString(&s_oled_handle, 0, 4, line, 16);
}

/**
 * @brief 声光报警控制
 */
void EnvCar_Alarm_Control(Alarm_Type_Enum alarm_type, bool enable)
{
    if (enable) {
        /* 步骤1：按报警类型启动对应声光（勿在中断里调用） */
        switch (alarm_type) {
            case ALARM_TYPE_OBSTACLE:
                Blue_Twinkle(3);
                Beep1_Alarm(3);
                break;

            case ALARM_TYPE_GAS_HIGH:
                Red_TurnOn();
                Beep2_TurnOn();
                break;

            case ALARM_TYPE_GAS_LOW:
                Red_TurnOn();
                Beep2_TurnOn();
                break;

            case ALARM_TYPE_SYSTEM_ERROR:
                Red_Twinkle(5);
                Blue_Twinkle(5);
                break;

            default:
                break;
        }
    } else {
        /* 步骤2：关闭报警 — 统一关断灯与蜂鸣器 */
        Red_TurnOff();
        Green_TurnOff();
        Blue_TurnOff();
        Beep1_TurnOff();
        Beep2_TurnOff();
    }
}

/**
 * @brief 无线通讯数据上传任务
 * @note 占位：周期到达 → 组包（JSON 等）→ JC278A_Send_Data。
 */
void EnvCar_Wireless_Upload_Data(void)
{
    /* 步骤1（占位）：判断是否到达上传周期 */
    /* 步骤2（占位）：填充传感器与状态字段到缓冲 */
    /* 步骤3（占位）：调用模块发送接口 */

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
 * @note 占位：非阻塞收 → 按关键字解析 → 调 EnvCar_Set_Mode / Manual_Control 等。
 */
void EnvCar_Wireless_Parse_Command(void)
{
    /* 步骤1（占位）：JC278A_Receive_Data 读入缓冲 */
    /* 步骤2（占位）：按行或前缀解析 MODE/SPEED/DIST/GAS/STOP 等 */
    /* 步骤3（占位）：调用应用层 API 生效 */

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
    /* 步骤1：保存目标模式 */
    g_EnvCar_Config.mode = mode;

    /* 步骤2：切入停止模式时立即停电机 */
    if (mode == ENVCAR_MODE_STOP) {
        TB6612_Stop();
    }
}

/**
 * @brief 紧急停止
 */
void EnvCar_Emergency_Stop(void)
{
    /* 步骤1：双轮驱动置零 */
    TB6612_Stop();
}

/**
 * @brief 手动控制小车运动
 */
void EnvCar_Manual_Control(int16_t left_speed, int16_t right_speed)
{
    /* 步骤1：仅手动模式下接受速度指令 */
    if (g_EnvCar_Config.mode == ENVCAR_MODE_MANUAL) {
        TB6612_SetSpeed(left_speed, right_speed);
    }
}
