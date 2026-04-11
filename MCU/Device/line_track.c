/**
 ******************************************************************************
 * @file    line_track.c
 * @brief   五路数字红外循迹 GPIO 读取（Gay1~Gay5）
 ******************************************************************************
 */

#include "line_track.h"

/**
 * @brief  单路是否为“在线上”
 */
bool LineTrack_ChannelIsLine(GPIO_TypeDef *port, uint16_t pin)
{
	return (HAL_GPIO_ReadPin(port, pin) == LINETRACK_LINE_ACTIVE_LEVEL);
}

/**
 * @brief  读取五路原始状态，bit0..bit4 对应 Gay1..Gay5
 */
uint8_t LineTrack_ReadRaw5(void)
{
	uint8_t m = 0U;
	if (LineTrack_ChannelIsLine(Gay1_GPIO_Port, Gay1_Pin)) {
		m |= (1U << 0);
	}
	if (LineTrack_ChannelIsLine(Gay2_GPIO_Port, Gay2_Pin)) {
		m |= (1U << 1);
	}
	if (LineTrack_ChannelIsLine(Gay3_GPIO_Port, Gay3_Pin)) {
		m |= (1U << 2);
	}
	if (LineTrack_ChannelIsLine(Gay4_GPIO_Port, Gay4_Pin)) {
		m |= (1U << 3);
	}
	if (LineTrack_ChannelIsLine(Gay5_GPIO_Port, Gay5_Pin)) {
		m |= (1U << 4);
	}
	return m;
}

/**
 * @brief  五路映射为左/中/右三路逻辑
 */
void LineTrack_ApplyToLogic(bool *ir_left, bool *ir_center, bool *ir_right)
{
	uint8_t m = LineTrack_ReadRaw5();
	bool g1 = ((m >> 0) & 1U) != 0U;
	bool g2 = ((m >> 1) & 1U) != 0U;
	bool g3 = ((m >> 2) & 1U) != 0U;
	bool g4 = ((m >> 3) & 1U) != 0U;
	bool g5 = ((m >> 4) & 1U) != 0U;

	if (ir_left != NULL) {
		*ir_left = (g1 || g2);
	}
	if (ir_center != NULL) {
		*ir_center = g3;
	}
	if (ir_right != NULL) {
		*ir_right = (g4 || g5);
	}
}
