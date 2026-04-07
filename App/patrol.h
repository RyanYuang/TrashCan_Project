/*
 * patrol.h
 *
 *  Created on: Feb 23, 2026
 *      Author: auto-generated
 *
 *  方形巡逻路径模块（非阻塞状态机）
 *  - 小车沿方形路径巡逻
 *  - 遇到障碍物（超声波 < 20cm）停车等待，障碍物消失后恢复前进
 */

#ifndef __PATROL_H__
#define __PATROL_H__

#include "main.h"

/* ======================== 可调参数 ======================== */
#define PATROL_SPEED             350    // 巡逻直行速度 (max 700)
#define PATROL_TURN_SPEED        700    // 巡逻转弯速度

#define PATROL_FORWARD_TIME_MS   2000   // 方形每边前进时间 (ms)
#define PATROL_TURN_TIME_MS      600    // 90°转弯时间 (ms)，需实测校准

#define OBSTACLE_THRESHOLD_CM    20     // 障碍物检测阈值 (cm)
/* ========================================================= */

/**
 * @brief 巡逻状态枚举
 *
 * 避障逻辑：前进中检测到障碍物 → 停车等待 → 障碍物消失 → 恢复前进
 */
typedef enum {
    PATROL_STATE_IDLE = 0,        // 空闲，未启动
    PATROL_STATE_FORWARD,         // 直行（方形的一条边）
    PATROL_STATE_TURN,            // 转弯（方形的拐角）
    PATROL_STATE_AVOID_STOP,      // 避障-停车等待障碍物消失
} Patrol_State_t;

/**
 * @brief 初始化巡逻模块
 */
void Patrol_Init(void);

/**
 * @brief 启动巡逻
 */
void Patrol_Start(void);

/**
 * @brief 停止巡逻
 */
void Patrol_Stop(void);

/**
 * @brief 巡逻任务（在主循环中反复调用，非阻塞）
 */
void Patrol_Task(void);

/**
 * @brief 获取当前巡逻状态
 * @return 当前状态
 */
Patrol_State_t Patrol_GetState(void);

/**
 * @brief 判断当前是否检测到障碍物（正处于避障停车状态）
 * @return 1=有障碍物, 0=无障碍物
 */
uint8_t Patrol_IsObstacle(void);

#endif /* __PATROL_H__ */
