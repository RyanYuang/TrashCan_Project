/*
 * OLED2.h
 *
 *  Created on: Nov 27, 2025
 *      Author: zeng
 */
#ifndef INC_OLED2_H_
#define INC_OLED2_H_


void OLED2_Init(void);
void OLED2_Clear(void);
void OLED2_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED2_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED2_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED2_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED2_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED2_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED2_ShowFloat(uint8_t Line, uint8_t Column, float Number, uint8_t Length, uint8_t Decimal_Places);



#endif /* INC_OLED2_H_ */
