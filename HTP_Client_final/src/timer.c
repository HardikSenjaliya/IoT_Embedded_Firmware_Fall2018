/*
 * timer.c
 *
 *  Created on: 20-Sep-2018
 *      Author: hardi
 */

#include "timer.h"

//Global varibale for timer0 interrupt flag
extern uint8_t scheduled_event;
/**
 * @brief  Setup timer for the required functionality
 * @param  void
 * @retval no return value
 */

void setup_timer0()
{
	//Set timer0 initialization structure params
	set_timer0_init_params(&timer0_init);

	//Initialize timer0
	TIMER_Init(TIMER0, &timer0_init);

	//Calculate prescaler value for 80 ms time period
	set_prescaler();

	//TIMER0 interrupt configuration
	timer0_interrupt_config();

}

/**
 * @brief  set timer0 initialization structure values to the required one
 * @param  void
 * @retval no return value
 */

void set_timer0_init_params(TIMER_Init_TypeDef* timer0_init)
{
	timer0_init->enable = false;
	timer0_init->debugRun = false;
	timer0_init->mode = timerModeDown;
	timer0_init->oneShot = true;
	timer0_init->clkSel = timerClkSelHFPerClk;
}

/**
 * @brief  calculates prescaler value and the counter value for the timer0
 * 		   clock and frequency
 * @param  void
 * @retval no return value
 */

void set_prescaler()
{
	uint32_t counter = 0;
	uint32_t prescaler = 0;
	float time_period = 0.080;
	float total_count = 0;
	uint32_t updated_freq;
	uint32_t timer_counter = 0;

	uint32_t TIMER0_freq = CMU_ClockFreqGet(cmuClock_TIMER0);

	updated_freq = TIMER0_freq;

	do
	{
		total_count = TIMER0_freq * time_period;

		if (total_count > TIMER_MAX_COUNT)
		{
			counter++;
			updated_freq = updated_freq / 2;
			total_count = updated_freq * time_period;

		}
	} while (total_count > TIMER_MAX_COUNT);

	prescaler = counter;

	//set the prescaler to the timer0 control register
	TIMER0->CTRL |= (prescaler << TIMER0_PRESCO_BIT);

	//calculate the counter value for timer0
	if (prescaler > 0)
	{
		timer_counter = (time_period * TIMER0_freq) / pow(2, prescaler);
	}
	else
	{
		timer_counter = (time_period * TIMER0_freq);
	}

	//write the counter value into the timer0 TOP register
	TIMER0->TOP = timer_counter;
}

/**
 * @brief  enables underflow interrupt flags and related interrupt configurations
 * @param  void
 * @retval no return value
 */

void timer0_interrupt_config()
{
	//Get all the pending and enabled interrupts
	uint32_t pending_int_flags = TIMER_IntGetEnabled(TIMER0);

	//clear all the pending and enabled interrupts
	TIMER_IntClear(TIMER0, pending_int_flags);

	//Set Underflow interrupts flag for tinmer0
	uint32_t underflow_int_bit = 0b000010;

	//Enable the Underflow interrupt
	TIMER_IntEnable(TIMER0, underflow_int_bit);

	NVIC_EnableIRQ(TIMER0_IRQn);

}

void TIMER0_IRQHandler()
{
	uint32_t pending_int_flags = TIMER_IntGet(TIMER0);

	CORE_ATOMIC_IRQ_DISABLE();

	TIMER_IntClear(TIMER0, pending_int_flags);

	if (pending_int_flags & TIMER_IF_UF)
	{
		//scheduled_event |= TIMER0_EVENT_FLAG;
		gecko_external_signal(TIMER0_EVENT_FLAG);
	}

	CORE_ATOMIC_IRQ_ENABLE();

}

