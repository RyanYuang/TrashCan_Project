/*
 * aht30.h
 *
 *  Created on: Nov 28, 2025
 *      Author: zeng
 */

#ifndef INC_AHT30_H_
#define INC_AHT30_H_

#include "i2c.h"

void AHT30_Init(void);
void AHT30_Read(float *Temperature, float *Humidity);
void AHT302_Init(void);
void AHT302_Read(float *Temperature, float *Humidity);

#endif /* INC_AHT30_H_ */
