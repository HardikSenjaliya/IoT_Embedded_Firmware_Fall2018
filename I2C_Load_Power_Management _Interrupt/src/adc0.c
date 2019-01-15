/*
 * moisture_adc.c
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 *
 */

#include "adc0.h"

#define ADC_DATA_AVAILABLE			(0x10)


/*Global Variable*/
uint32_t MOISTURE_reading[100] = {0};

extern volatile uint8_t external_event_scheduler;

void soil_moisture_adc0_init(void)
{

	// Clear the single FIFO
	ADC0->SINGLEFIFOCLEAR = ADC_SINGLEFIFOCLEAR_SINGLEFIFOCLEAR;

	// Enable ADC0 clock source
	/* Setting up the AUXHFRCO clock */
	CMU_AUXHFRCOBandSet(CMU_AUXHFRCO_MIN);
	CMU_OscillatorEnable(cmuOsc_AUXHFRCO, true, true);
	CMU_ClockSelectSet(cmuClock_AUX, cmuSelect_AUXHFRCO);
	ADC0->CTRL |= ADC_CTRL_ADCCLKMODE_ASYNC | ADC_CTRL_ASYNCCLKEN_ASNEEDED;

	//Setting up the ADCCTRL register and setting up the ADC clock to AUXHFRCO
	CMU->ADCCTRL  = CMU_ADCCTRL_ADC0CLKSEL_AUXHFRCO;

	CMU_ClockSelectSet(cmuClock_ADC0, cmuSelect_AUXHFRCO);

	/* Enable ADC clock */
	CMU_ClockEnable(cmuClock_ADC0, true);

	/* Disabling the PA0 as it is needed for the soil moisture input */
	GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 0);

	ADC_Init_TypeDef init = ADC_INIT_DEFAULT;
	init.timebase = ADC_TimebaseCalc(0);
	init.prescale = ADC_PrescaleCalc(cmuClock_AUX, 0);

	/* Initialize for single channnel conversion */
	ADC_InitSingle_TypeDef init_single = ADC_INITSINGLE_DEFAULT;
	init_single.acqTime = adcAcqTime32;
	init_single.diff = false;
	init_single.prsSel = adcPRSSELCh0;
	init_single.reference = adcRefVDD;
	init_single.posSel = adcPosSelAPORT3XCH8;
	init_single.negSel = adcNegSelVSS;
	init_single.prsEnable = false;
	init_single.fifoOverwrite = true;
	init_single.rep = true;

	/*Bias current programming*/
	ADC0->BIASPROG |= ADC_BIASPROG_GPBIASACC_LOWACC | ADC_BIASPROG_ADCBIASPROG_SCALE8;

	ADC_Init(ADC0, &init);
	ADC_InitSingle(ADC0, &init_single);
}

void soil_moisture_adc0_setup(void)
{
	printf("Moisture sensor setup\r\n");

	/*Configure Single channel ADC0*/
	soil_moisture_adc0_init();

	/*ADC0 interrupt setup*/
	ADC_IntEnable(ADC0, ADC_IEN_SINGLE);

	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);

	printf("Starting ADC0 \r\n");
	ADC_Start(ADC0, adcStartSingle);

}

void ADC0_IRQHandler(void)
{
	static uint8_t i = 0;
	CORE_AtomicDisableIrq();

	uint32_t int_flags = ADC_IntGet(ADC0);

	ADC_IntClear(ADC0, int_flags);

	/*Read the mositure data value from the FIFO*/
	if(ADC0->IF & ADC_IF_SINGLE)
	{
		printf("ADC interrupt\r\n");
		MOISTURE_reading[i++] = ADC_DataSingleGet(ADC0);
		//external_event_scheduler |= ADC_DATA_AVAILABLE;
		//gecko_external_signal(external_event_scheduler);
	}

	CORE_AtomicEnableIrq();
}
