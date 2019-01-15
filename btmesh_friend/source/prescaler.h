/*
 * prescaler.h
 *
 *  Created on: 11-Dec-2018
 *      Author: hardi
 */

#ifndef SOURCE_PRESCALER_H_
#define SOURCE_PRESCALER_H_

#include <stdint.h>
#include <math.h>
#include "em_cmu.h"

void calculate_prescaler(uint8_t, uint8_t);
uint32_t calculate_COMP0_Count(uint8_t, uint8_t);

//uint32_t calculate_COMP1_Count(Energy_Modes, uint32_t, float);


#endif /* SOURCE_PRESCALER_H_ */
