/*
 * sleep_routine.h
 *
 *  Created on: 14-Sep-2018
 *      Author: hardi
 */

#include <stdint.h>

#include "stdbool.h"
#include "em_emu.h"
#include "em_core.h"
#include "sleep.h"

#ifndef SLEEP_ROUTINE_H_
#define SLEEP_ROUTINE_H_

typedef enum
{
	EM0 = 0,
	EM1 = 1,
	EM2 = 2,
	EM3 = 3,
	EM4 = 4

}Energy_Modes;


void block_sleep_mode(Energy_Modes);
void unblock_sleep_mode(Energy_Modes);
void enter_sleep_routine();


#endif /* SLEEP_ROUTINE_H_ */
