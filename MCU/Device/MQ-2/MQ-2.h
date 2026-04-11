#ifndef _MQ_2_H_
#define _MQ_2_H_

/*
 * 硬件与 Cube 对齐说明（测试前必读）
 * ---------------------------------------------------------------------------
 * - 当前工程 ADC1 规则通道为 ADC_CHANNEL_0，对应引脚 PA0（ADC1_IN0）。
 * - 若 MQ-2 模块模拟输出接在 PA4（IN4）或 PA5（IN5），请在 CubeMX 中改通道并重新生成，
 *   或同步修改 Core/Src/adc.c 中 HAL_ADC_ConfigChannel，否则读数不是 MQ2。
 * - 模块常见为 5V 供电、模拟输出可达 0~5V：须确认已分压/限幅至 MCU ADC 允许范围（通常 ≤3.3V），
 *   切勿将 5V 直接进 ADC 引脚。
 * ---------------------------------------------------------------------------
 */

#include "main.h"
#include <stdint.h>

float MQ2ConvertPPM(uint32_t adc_data);

/**
 * @brief  读取 MQ-2 所在 ADC 通道的 12bit 原始值（0~4095）
 * @note   使用阻塞采样；首次调用会执行一次 F1 ADC 校准。须与 MX_ADC1_Init 配置的通道一致。
 * @retval 转换成功为 ADC 码值；失败返回 0
 */
uint32_t MQ2_ReadRaw(void);

#endif
