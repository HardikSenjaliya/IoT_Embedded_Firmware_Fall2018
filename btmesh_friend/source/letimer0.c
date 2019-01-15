/*
 * letimer0.c
 *
 *  Created on: 11-Dec-2018
 *      Author: hardi
 */

#include "letimer0.h"

#define LETIMER0_INTERRUPT_EVENT					(11)

/**
 * @brief  Define letimer0 parameters
 * @param  EMx: Lowest energy mode
 * 		led_on_time: led on time period
 * 		time_period: total time period
 * @retval no return value
 */

void setup_letimer0(uint8_t EMx, uint8_t time_period)
{

	/*clock setup*/
	CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockEnable(cmuClock_LETIMER0, true);


	//Configure letimer0
	set_letimer_init_params(&letimer_init);

	//Initialize letimer0
	LETIMER_Init(LETIMER0, &letimer_init);

	//Calculate prescaler for the clock and led on time counter
	calculate_prescaler(EMx, time_period);

	//Configure letimer0 interrupt
	letimer0_interrupt_config(LETIMER0, EMx, time_period);

}

/**
 * @brief  Define letimer0 parameters
 * @param  letimer_init: a pointer to LETIMER_Init_Typedef
 * @retval no return value
 */

void set_letimer_init_params(LETIMER_Init_TypeDef* letimer_init)
{
	letimer_init->enable = false;
	letimer_init->debugRun = false;
	letimer_init->comp0Top = true;
	letimer_init->bufTop = false;
	letimer_init->out0Pol = 0;
	letimer_init->out1Pol = 0;
}

/**
 * @brief  Configure letimer0 interrupt registers and enable interrupts
 * @param  letimer0: a pointer to LETIMER_Typedef
 * 		EMx: lowest possible energy mode
 * 		on_time: LED on time
 * 		timer_period: Time period of a led
 * @retval no return value
 */

void letimer0_interrupt_config(LETIMER_TypeDef* letimer0, uint8_t EMx, uint8_t time_period)
{


	//Calculate COMP0 counter value
	uint32_t COMP0_Count_value = calculate_COMP0_Count(EMx, time_period);

	//Calculate COMP1 counter value
	//uint32_t COMP1_Count_value = calculate_COMP1_Count(EMx, COMP0_Count_value, on_time);

	//get all the enabled and pending letimer0 interrupt
	uint32_t pending_int_flags = LETIMER_IntGetEnabled(LETIMER0);

	//Enable comp0 and comp1 interrupts
	uint32_t set_comp_uf_int_flag = 0b00001;

	//Clear all the pending and enabled interrupts
	LETIMER_IntClear(LETIMER0, pending_int_flags);

	//Load COMP0 and COMP1 counter values
	LETIMER_CompareSet(LETIMER0, 0, COMP0_Count_value);
	//LETIMER_CompareSet(LETIMER0, 1, COMP1_Count_value);

	//Enable letimer0 interrupts in the IEN register
	LETIMER_IntEnable(LETIMER0, set_comp_uf_int_flag);
	//while(LETIMER0 -> SYNCBUSY == 1);

	//Enable letimer0 interrupt in NVIC
	NVIC_EnableIRQ(LETIMER0_IRQn);
}

/**
 * @brief  ISR for letimer0
 * @param  no params
 * @retval no return value
 */
void LETIMER0_IRQHandler()
{

	//Get all the peding interrupt flags
	uint32_t interrupt_Flag = LETIMER_IntGet(LETIMER0);

	//Disable all the interrupts for atomic handling
	CORE_ATOMIC_IRQ_DISABLE();

	//Clear all the pending interrupts
	LETIMER_IntClear(LETIMER0, interrupt_Flag);

	//If timer underflow interrupt occurs set the schedular flag
	// and call the scheduler to perform required task
	if (LETIMER_IF_UF & interrupt_Flag)
	{
		gecko_external_signal(LETIMER0_INTERRUPT_EVENT);
	}

	CORE_ATOMIC_IRQ_ENABLE();
}
