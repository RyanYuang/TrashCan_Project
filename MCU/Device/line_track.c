/**
 ******************************************************************************
 * @file    line_track.c
 * @brief   四路数字红外循迹 GPIO 读取（Gay1~Gay4）
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
 * @brief  读取四路原始状态，bit0..bit3 对应 Gay1..Gay4
 */
uint8_t LineTrack_ReadRaw4(void)
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
	return m;
}

uint8_t LineTrack_ReadRaw5(void)
{
	return LineTrack_ReadRaw4();
}

/**
 * @brief  Gay1 最右…Gay4 最左 → 映射为 bit0..3 车体从左到右
 */
uint8_t LineTrack_ReadSpatial4(void)
{
	uint8_t hw = LineTrack_ReadRaw4();
	bool r_out = ((hw >> 0) & 1U) != 0U; /* Gay1 */
	bool r_mid = ((hw >> 1) & 1U) != 0U; /* Gay2 */
	bool l_mid = ((hw >> 2) & 1U) != 0U; /* Gay3 */
	bool l_out = ((hw >> 3) & 1U) != 0U; /* Gay4 */
	uint8_t m = 0U;
	if (l_out) {
		m |= (1U << 0);
	}
	if (l_mid) {
		m |= (1U << 1);
	}
	if (r_mid) {
		m |= (1U << 2);
	}
	if (r_out) {
		m |= (1U << 3);
	}
	return m;
}

/**
 * @brief  四路映射为左/中/右三路逻辑（供状态上报与调试）
 */
void LineTrack_ApplyToLogic(bool *ir_left, bool *ir_center, bool *ir_right)
{
	uint8_t s = LineTrack_ReadSpatial4();
	bool l_out = ((s >> 0) & 1U) != 0U;
	bool l_mid = ((s >> 1) & 1U) != 0U;
	bool r_mid = ((s >> 2) & 1U) != 0U;
	bool r_out = ((s >> 3) & 1U) != 0U;

	if (ir_left != NULL) {
		*ir_left = l_out;
	}
	if (ir_center != NULL) {
		*ir_center = (l_mid || r_mid);
	}
	if (ir_right != NULL) {
		*ir_right = r_out;
	}
}
