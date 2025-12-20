/*
 * beep.h
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */

#ifndef INC_BEEP_H_
#define INC_BEEP_H_

#include "gpio.h"
#include "Beep_config.h"
void Beep1_TurnOn(void);
void Beep1_TurnOff(void);
void Beep1_Alarm(unsigned char count);

void Beep2_TurnOn(void);
void Beep2_TurnOff(void);
void Beep2_Alarm(unsigned char count);

#endif /* INC_BEEP_H_ */
