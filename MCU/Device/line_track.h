/**
 ******************************************************************************
 * @file    line_track.h
 * @brief   四路数字红外循迹（Cube 标签 Gay1~Gay4）：GPIO 数字量读取
 * @note    硬件引脚 Gay1~Gay4 在车上为：Gay1 最右、Gay2 右靠中、Gay3 左靠中、Gay4 最左。
 *          循迹算法使用 LineTrack_ReadSpatial4()：bit0..3 = 车体从左到右 外左、左中、右中、外右。
 *          LineTrack_ReadRaw4() 仍为 Gay1→bit0 … Gay4→bit3，便于对照原理图。电平极性见 LINETRACK_LINE_ACTIVE_LEVEL。
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
 * @brief  读取四路原始状态，bit0..bit3 对应 Gay1..Gay4（Cube 同名脚），在线上为 1
 * @retval 低 4 位有效
 */
uint8_t LineTrack_ReadRaw4(void);

/**
 * @brief  车体坐标四路：bit0 外左(Gay4)、bit1 左中(Gay3)、bit2 右中(Gay2)、bit3 外右(Gay1)
 */
uint8_t LineTrack_ReadSpatial4(void);

/**
 * @brief  兼容旧接口：等价于 LineTrack_ReadRaw4()（仅低 4 位有效）
 */
uint8_t LineTrack_ReadRaw5(void);

/**
 * @brief  将四路映射为应用层三路：左=最左外(Gay4)、中=左中|右中(Gay3|Gay2)、右=最右外(Gay1)
 */
void LineTrack_ApplyToLogic(bool *ir_left, bool *ir_center, bool *ir_right);

#ifdef __cplusplus
}
#endif

#endif /* DEVICE_LINE_TRACK_H */
