/*
 * timer.h
 *
 *  Created on: 20-Sep-2018
 *      Author: hardi
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <math.h>
#include "stdbool.h"
#include "em_timer.h"
#include "em_cmu.h"
#include "gpio.h"
#include "sleep_routine.h"
#include "native_gecko.h"

#define TIMER_MAX_COUNT				(65535)
#define TIMER0_PRESCO_BIT			(24)
#define TIMER0_EVENT_FLAG			(0x02)

TIMER_Init_TypeDef timer0_init;

void setup_timer0();
void set_timer0_init_params(TIMER_Init_TypeDef*);
void set_prescaler();
void timer0_interrupt_config();
void TIMER0_IRQHandler();
#endif /* TIMER_H_ */
