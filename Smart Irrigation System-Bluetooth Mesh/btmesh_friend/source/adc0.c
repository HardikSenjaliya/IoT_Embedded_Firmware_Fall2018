/*
 * moisture_adc.c
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 *
 */

#include <adc0.h>
#include "display_interface.h"
#include "led.h"

#define ADC_DATA_AVAILABLE					(10)
#define ADC0_12BIT_MAX_VALUE				(4095)
#define SPRINKLER_STATE_SHIFT				(8)
#define SPRINKLER_STATE_ON					(1)
#define SPRINKLER_STATE_OFF					(0)
#define LED_OFF							  	(0)
#define LED_ON							  	(1)




/*Global Variable*/
uint32_t soil_moisture_raw_value = 0;

extern volatile uint8_t external_event_scheduler;

void adc0_clock_setup(void)
{
	/* Setting up the AUXHFRCO clock */
	CMU_AUXHFRCOBandSet(CMU_AUXHFRCO_MIN);

	CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);

	CMU_ClockSelectSet(cmuClock_AUX, cmuSelect_AUXHFRCO);

	/*Setting ADC clock to asynck mode for EM2 mode operation*/
	ADC0->CTRL |= ADC_CTRL_ADCCLKMODE_ASYNC | ADC_CTRL_ASYNCCLKEN_ASNEEDED;

	//Setting up the ADCCTRL register and setting up the ADC clock to AUXHFRCO
	CMU->ADCCTRL = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;

	CMU_ClockSelectSet(cmuClock_ADC0, cmuSelect_AUXHFRCO);

	/*Enable Clock for ADC0*/
	CMU_ClockEnable(cmuClock_ADC0, true);

}

void soil_moisture_adc0_init(void)
{

	ADC_Init_TypeDef init = ADC_INIT_DEFAULT;

	init.timebase = ADC_TimebaseCalc(0);

	init.prescale = ADC_PrescaleCalc(cmuClock_AUX, 0);

	//Initialize for single channnel conversion
	ADC_InitSingle_TypeDef init_single = ADC_INITSINGLE_DEFAULT;

	init_single.acqTime = adcAcqTime32;
	init_single.diff = false;
	init_single.prsSel = adcPRSSELCh0;
	init_single.reference = adcRefVDD;
	init_single.posSel = adcPosSelAPORT1YCH19;
	init_single.negSel = adcNegSelVSS;
	init_single.prsEnable = false;
	init_single.fifoOverwrite = true;
	init_single.rep = true;

	/*Bias current programming*/
	ADC0->BIASPROG |= ADC_BIASPROG_GPBIASACC_LOWACC
			| ADC_BIASPROG_ADCBIASPROG_SCALE8;

	/*Clear the single FIFO*/
	ADC0->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;

	ADC_Init(ADC0, &init);

	ADC_InitSingle(ADC0, &init_single);
}

void soil_moisture_adc0_setup(void)
{
	/*adc0 clock setup*/
	adc0_clock_setup();

	/*Configure Single channel ADC0*/
	soil_moisture_adc0_init();

	/*ADC0 interrupt setup
	 ADC_IntEnable(ADC0, ADC_IEN_SINGLE);

	 NVIC_ClearPendingIRQ(ADC0_IRQn);
	 NVIC_EnableIRQ(ADC0_IRQn);
	 */

	printf("Starting ADC0 \r\n");
	ADC_Start(ADC0, adcStartSingle);
}

uint32_t get_soil_moisture_calibrated_value(uint32_t moisture_level)
{
	return ((moisture_level * 100) / ADC0_12BIT_MAX_VALUE);
}



/*NOTE:
 * If we use adc with interrupt then interrupt is so fast
 * that the application misses some command in between and
 * is not working properly
 * for eg. if we start adc in the gecko_boot id then not a single event will occour
 * and application will hang in the ADC_irq handler, in fact Mesh smartphone app can't even
 * detect the Node because of that even provisioning is not possible
 * and if we try to start adc after provisioning then application will miss some commands and
 * will behave not as expected*/

/*

 void ADC0_IRQHandler(void)
 {


 CORE_AtomicDisableIrq();

 uint32_t int_flags = ADC_IntGet(ADC0);

 ADC_IntClear(ADC0, int_flags);

 Read the mositure data value from the FIFO
 if(ADC0->IF & ADC_IF_SINGLE)
 {
 MOISTURE_reading = ADC_DataSingleGet(ADC0);
 sprintf(buf, "%lu", MOISTURE_reading);
 DI_Print(buf, 7);
 gecko_external_signal(ADC_DATA_AVAILABLE);
 }

 CORE_AtomicEnableIrq();
 }
 */
