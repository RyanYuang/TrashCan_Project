/*
 * rgb.h
 *
 *  Created on: Dec 1, 2025
 *      Author: zeng
 */

#ifndef RGB_H_
#define RGB_H_

#include "gpio.h"

void Red_TurnOn(void);
void Red_TurnOff(void);
void Red_Twinkle(unsigned char count);
void Green_TurnOn(void);
void Green_TurnOff(void);
void Green_Twinkle(unsigned char count);
void Blue_TurnOn(void);
void Blue_TurnOff(void);
void Blue_Twinkle(unsigned char count);

#endif /* RGB_H_ */
