/*
 * prescaler.c
 *
 *  Created on: 11-Dec-2018
 *      Author: hardi
 */

#include "prescaler.h"

#define LETIMER_MAX_COUNT 					(65535)
#define LFXO_FREQUENCY 						(32768)
#define ULFRCO_FREQUENCY 					(1000)

uint32_t prescaler;

/**
 * @brief  Calculate prescaler for the required clock
 * @param  EMx: Lowest possible energy mode
 * 		time_period: led time period
 * @retval no return value
 */

void calculate_prescaler(uint8_t EMx, uint8_t time_period)
{
	uint32_t total_count = 0, updated_freq = 0, counter = 0;

	if (EMx < 3)
	{
		updated_freq = LFXO_FREQUENCY;
		do
		{
			total_count = LFXO_FREQUENCY * time_period;
			if (total_count > LETIMER_MAX_COUNT)
			{
				counter++;
				updated_freq = updated_freq / 2;
				total_count = updated_freq * time_period;

			}
		} while (total_count > LETIMER_MAX_COUNT);
	}
	else
	{
		updated_freq = ULFRCO_FREQUENCY;
		do
		{
			total_count = ULFRCO_FREQUENCY * time_period;
			if (total_count > LETIMER_MAX_COUNT)
			{
				counter++;
				updated_freq = updated_freq / 2;
				total_count = updated_freq * time_period;

			}
		} while (total_count > LETIMER_MAX_COUNT);
	}

	prescaler = counter;
	CMU->LFAPRESC0 = prescaler;
}

/**
 * @brief  Calculate count value for COMP1 register
 * @param  EMx: Lowest energy mode
 * 		timer_period: led time period
 * @retval returns count for COMP1
 */

uint32_t calculate_COMP0_Count(uint8_t EMx, uint8_t time_period)
{
	uint32_t COMP0_Count = 0;
	if (EMx < 3)
	{
		if (prescaler > 0)
		{
			COMP0_Count = (time_period * LFXO_FREQUENCY) / pow(2,prescaler);
		}
		else
		{
			COMP0_Count = (time_period * LFXO_FREQUENCY);
		}
	}
	else
	{
		if (prescaler > 0)
		{
			COMP0_Count = (time_period * ULFRCO_FREQUENCY) / pow(2,prescaler);
		}
		else
		{
			COMP0_Count = (time_period * ULFRCO_FREQUENCY);
		}
	}

	return COMP0_Count;
}

/**
 * @brief  Calculate count value for COMP0 register
 * @param  EMx: Lowest energy mode
 * 		on_time: led on time period
 * 		comp0_count: count value of comp0
 * @retval returns count for COMP0
 */
/*

uint32_t calculate_COMP1_Count(uint8_t EMx, uint32_t comp0_count,
		uint8_t on_time)
{
	int COMP1_Count = 0;
	if (EMx < 3)
	{
		if (prescaler > 0)
		{
			int temp = (on_time * LFXO_FREQUENCY) / (prescaler << 1);
			COMP1_Count = comp0_count - temp;
		}
		else
		{
			int temp = (on_time * LFXO_FREQUENCY);
			COMP1_Count = comp0_count - temp;
		}
	}
	else
	{
		if (prescaler > 0)
		{
			int temp = (on_time * ULFRCO_FREQUENCY) / (prescaler << 1);
			COMP1_Count = comp0_count - temp;
		}
		else
		{
			int temp = (on_time * ULFRCO_FREQUENCY);
			COMP1_Count = comp0_count - temp;
		}
	}

	return COMP1_Count;
}
*/

