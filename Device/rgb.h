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
void UV_Open(void);
void UV_Close(void);
void Red2_TurnOn(void);
void Red2_TurnOff(void);
void Red2_Twinkle(unsigned char count);
void Green2_TurnOn(void);
void Green2_TurnOff(void);
void Green2_Twinkle(unsigned char count);
void Blue2_TurnOn(void);
void Blue2_TurnOff(void);
void Blue2_Twinkle(unsigned char count);
void UV2_Open(void);
void UV2_Close(void);
void UV_Key_Scan(void);

#endif /* RGB_H_ */
