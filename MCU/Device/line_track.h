/**
 ******************************************************************************
 * @file    line_track.h
 * @brief   五路数字红外循迹（Cube 标签 Gay1~Gay5）：GPIO 数字量读取
 * @note    引脚定义见 main.h（Gay1_Pin..Gay5_Pin）。电平极性由 LINETRACK_LINE_ACTIVE_LEVEL 配置。
 ******************************************************************************
 */

#ifndef DEVICE_LINE_TRACK_H
#define DEVICE_LINE_TRACK_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 读到该电平时视为“在黑线上”（按模块 D0 实际输出修改）。
 * 常见：比较器输出高=有黑线用 GPIO_PIN_SET；若为低有效则改为 GPIO_PIN_RESET。
 */
#ifndef LINETRACK_LINE_ACTIVE_LEVEL
#define LINETRACK_LINE_ACTIVE_LEVEL GPIO_PIN_SET
#endif

/**
 * @brief  单路是否为“在线上”
 */
bool LineTrack_ChannelIsLine(GPIO_TypeDef *port, uint16_t pin);

/**
 * @brief  读取五路原始状态，bit0..bit4 对应 Gay1..Gay5，在线上为 1
 * @retval 低 5 位有效
 */
uint8_t LineTrack_ReadRaw5(void);

/**
 * @brief  将五路映射为应用层三路：左区(Gay1|Gay2)、中(Gay3)、右区(Gay4|Gay5)
 * @note   面向车头从左到右 Gay1..Gay5；若安装方向相反可在此函数内交换逻辑
 */
void LineTrack_ApplyToLogic(bool *ir_left, bool *ir_center, bool *ir_right);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_LINE_TRACK_H */
