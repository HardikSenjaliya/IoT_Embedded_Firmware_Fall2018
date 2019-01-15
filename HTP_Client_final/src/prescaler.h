/*
 * prescaler_config.h
 *
 *  Created on: 15-Sep-2018
 *      Author: hardi
 */

#ifndef PRESCALER_H_
#define PRESCALER_H_

#include <stdint.h>
#include <math.h>
#include "em_cmu.h"
#include "sleep_routine.h"

void calculate_prescaler(Energy_Modes, float);
uint32_t calculate_COMP0_Count(Energy_Modes, float);
uint32_t calculate_COMP1_Count(Energy_Modes, uint32_t, float);

#endif /* PRESCALER_H_ */
