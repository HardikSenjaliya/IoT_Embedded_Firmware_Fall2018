/*
 * letimer0.h
 *
 *  Created on: 11-Dec-2018
 *      Author: hardi
 */

#ifndef SOURCE_LETIMER0_H_
#define SOURCE_LETIMER0_H_

#include "prescaler.h"
#include "em_letimer.h"
#include "stdbool.h"
#include "em_emu.h"
#include "em_core.h"
#include "native_gecko.h"

LETIMER_Init_TypeDef letimer_init;
LETIMER_TypeDef letimer0;

//function prototypes
void setup_letimer0(uint8_t, uint8_t);
void set_letimer_init_params(LETIMER_Init_TypeDef*);
void LETIMER0_IRQHandler();
void letimer0_interrupt_config(LETIMER_TypeDef*, uint8_t, uint8_t);

#endif /* SOURCE_LETIMER0_H_ */
