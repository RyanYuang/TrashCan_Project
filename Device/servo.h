/*
 * servo.h
 *
 *  Created on: Nov 29, 2025
 *      Author: zeng
 */

#ifndef INC_SERVO_H_
#define INC_SERVO_H_

#include "tim.h"
#include "stdbool.h"
#include "User_Main.h"

extern volatile bool servo1_state;  // 初始关
extern volatile bool servo2_state;  // 初始关
extern volatile bool voice_mode1;  // 初始非语音模式
extern volatile bool voice_mode2;  // 初始非语音模式

void servo_init(void);
void servo1_open(void);
void servo1_close(void);
void servo2_open(void);
void servo2_close(void);

#endif /* INC_SERVO_H_ */
