/*
 * patrol.c
 *
 *  Created on: Feb 23, 2026
 *      Author: auto-generated
 *
 *  方形巡逻路径模块 - 基于状态机的非阻塞实现
 *
 *  巡逻路线：
 *    前进 → 右转90° → 前进 → 右转90° → 前进 → 右转90° → 前进 → 右转90° (循环)
 *
 *  避障逻辑（超声波 < 20cm）：
 *    停车等待 → 障碍物消失 → 恢复前进
 */

#include "patrol.h"
#include "TB6612.h"
#include "ultrasonic.h"
#include "stdio.h"

/* ======================== 内部变量 ======================== */
static Patrol_State_t s_state = PATROL_STATE_IDLE;   // 当前状态
static uint32_t       s_tick_start = 0;              // 当前动作起始时刻 (ms)
static uint8_t        s_side_count = 0;              // 已走完的边数（0~3）

/* ======================== 内部函数 ======================== */

/**
 * @brief 获取当前毫秒级时间戳（基于 SysTick 定时中断）
 */
static inline uint32_t Patrol_GetTick(void)
{
    return HAL_GetTick();
}

/**
 * @brief 判断从 start 开始是否已经过了 duration_ms 毫秒
 */
static inline uint8_t Patrol_IsTimeout(uint32_t start, uint32_t duration_ms)
{
    return (Patrol_GetTick() - start) >= duration_ms;
}

/**
 * @brief 切换到新状态并记录起始时刻
 */
static void Patrol_SwitchState(Patrol_State_t new_state)
{
    s_state = new_state;
    s_tick_start = Patrol_GetTick();
}

/* ======================== 外部接口 ======================== */

/**
 * @brief 初始化巡逻模块
 */
void Patrol_Init(void)
{
    s_state = PATROL_STATE_IDLE;
    s_side_count = 0;
    s_tick_start = 0;
}

/**
 * @brief 启动方形巡逻
 */
void Patrol_Start(void)
{
    if (s_state == PATROL_STATE_IDLE) {
        s_side_count = 0;
        printf("[Patrol] Start\r\n");
        Patrol_SwitchState(PATROL_STATE_FORWARD);
        // 立即驱动电机前进
        TB6612_SetSpeed(PATROL_SPEED, PATROL_SPEED);
    }
}

/**
 * @brief 停止巡逻并让电机停转
 */
void Patrol_Stop(void)
{
    TB6612_Stop();
    s_state = PATROL_STATE_IDLE;
    printf("[Patrol] Stop\r\n");
}

/**
 * @brief 获取当前巡逻状态
 */
Patrol_State_t Patrol_GetState(void)
{
    return s_state;
}

/**
 * @brief 判断当前是否检测到障碍物
 * @return 1=有障碍物（正处于避障停车状态）, 0=无障碍物
 */
uint8_t Patrol_IsObstacle(void)
{
    int dist = ultrasonic_get_distance();
    if(dist > 5 && dist < OBSTACLE_THRESHOLD_CM)
    {
    	printf("dis = %d cm\r\n",dist);
    	return 1;
    }
    else
    {
    	return 0;
    }
}

/**
 * @brief 巡逻状态机任务（非阻塞，需在主循环中反复调用）
 *
 *  每次调用检查：
 *    1. 是否检测到障碍物 → 进入避障子流程
 *    2. 当前动作是否超时 → 切换到下一个动作
 */
void Patrol_Task(void)
{
	
    if (s_state == PATROL_STATE_IDLE) 
    {
        return;  // 未启动，直接返回
    }

    int dist = ultrasonic_get_distance();

    /* -------- 避障检测（仅在前进状态下触发） -------- */
    if (s_state == PATROL_STATE_FORWARD) {
        if (dist > 0 && dist < OBSTACLE_THRESHOLD_CM) {
            printf("[Patrol] Obstacle! dist=%d cm, stopping\r\n", dist);
            TB6612_Stop();
            Patrol_SwitchState(PATROL_STATE_AVOID_STOP);
            return;
        }
    }

    /* -------- 状态机主体 -------- */
    switch (s_state) {

    /* ---------- 正常巡逻 ---------- */
    case PATROL_STATE_FORWARD:
        // 时间到 → 转弯
        if (Patrol_IsTimeout(s_tick_start, PATROL_FORWARD_TIME_MS)) {
            printf("[Patrol] Side %d done, turning\r\n", s_side_count);
            // 原地右转：左轮正转，右轮反转
            TB6612_SetSpeed(PATROL_TURN_SPEED, -PATROL_TURN_SPEED);
            Patrol_SwitchState(PATROL_STATE_TURN);
        }
        break;

    case PATROL_STATE_TURN:
        // 转弯时间到 → 前进（下一条边）
        if (Patrol_IsTimeout(s_tick_start, PATROL_TURN_TIME_MS)) {
            s_side_count++;
            if (s_side_count >= 4) {
                s_side_count = 0;  // 完成一圈，重新计数
                printf("[Patrol] Square complete, restart\r\n");
            }
            TB6612_SetSpeed(PATROL_SPEED, PATROL_SPEED);
            Patrol_SwitchState(PATROL_STATE_FORWARD);
        }
        break;

    /* ---------- 避障：停车等待障碍物消失 ---------- */
    case PATROL_STATE_AVOID_STOP:
        // 持续检测距离，障碍物消失后（距离 >= 阈值 或 无效读数）恢复前进
        if (dist <= 0 || dist >= OBSTACLE_THRESHOLD_CM) {
            printf("[Patrol] Obstacle cleared, resume forward\r\n");
            TB6612_SetSpeed(PATROL_SPEED, PATROL_SPEED);
            Patrol_SwitchState(PATROL_STATE_FORWARD);
        }
        // 否则继续停车等待，什么都不做
        break;

    default:
        break;
    }
}
