/*
 * sleep_routine.c
 *
 * @credit Used from the Lecture Slides by Prof Keith Graham
 * Lecture #4 dated 09/06/2018
 *
 *
 */
/***************************************************************************//**
* @file sleep_routine.c
*******************************************************************************
* @section License
* <b>(C) Copyright 2015 Silicon Labs, http://www.silabs.com</b>
*******************************************************************************
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
*
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*
* DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Silicon Labs has no
* obligation to support this Software. Silicon Labs is providing the
* Software "AS IS", with no express or implied warranties of any kind,
* including, but not limited to, any implied warranties of merchantability
* or fitness for any particular purpose or warranties against infringement
* of any proprietary rights of a third party.
*
* Silicon Labs will not be liable for any consequential, incidental, or
* special damages, or any other relief, or for any claim by any third party,
* arising from your use of this Software.
*
******************************************************************************/

#include "sleep_routine.h"
#include "sleep.h"

static uint8_t sleep_block_counter[5];

/**
 * @brief  Blocks required energy modes
 * @param  EMx: increment counter to block EMx and below energy modes
 * @retval no return value
 */
void block_sleep_mode(Energy_Modes EMx)
{
	CORE_AtomicDisableIrq();
	sleep_block_counter[EMx]++;
	CORE_AtomicEnableIrq();
}

/**
 * @brief  unblocks required energy modes
 * @param  EMx: decrement counter to unblock EMx and below energy modes
 * @retval no return value
 */
void unblock_sleep_mode(Energy_Modes EMx)
{
	CORE_AtomicDisableIrq();
	sleep_block_counter[EMx]--;
	CORE_AtomicEnableIrq();
}

/**
 * @brief  Blocks required energy modes
 * @param  no params
 * @retval returns count for COMP1
 */
void enter_sleep_routine()
{
	if (sleep_block_counter[EM0] > 0)
	{
		return;
	}
	else if (sleep_block_counter[EM1] > 0)
	{
		return;
	}
	else if (sleep_block_counter[EM2] > 0)
	{
		EMU_EnterEM1();
	}
	else if (sleep_block_counter[EM3] > 0)
	{
		EMU_EnterEM2(true);
	}
	else
	{
		EMU_EnterEM3(true);
	}

}

