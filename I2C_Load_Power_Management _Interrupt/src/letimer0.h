/*
 * letimer0.h
 *
 *  Created on: 12-Sep-2018
 *      Author: hardi
 */

#ifndef LETIMER0_H_
#define LETIMER0_H_

#include <src/prescaler.h>
#include "em_letimer.h"
#include "gpio.h"
#include "sleep_routine.h"
#include "stdbool.h"
#include "em_emu.h"

LETIMER_Init_TypeDef letimer_init;
LETIMER_TypeDef letimer0;

//function prototypes
void setup_letimer0(Energy_Modes, float, float);
void set_letimer_init_params(LETIMER_Init_TypeDef*);
void LETIMER0_IRQHandler();
void letimer0_interrupt_config(LETIMER_TypeDef*, Energy_Modes, float, float);

#endif /* LETIMER0_H_ */
