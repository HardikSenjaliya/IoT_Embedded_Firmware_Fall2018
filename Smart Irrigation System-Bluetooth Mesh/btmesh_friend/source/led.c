/*
 * led.c
 *
 *  Created on: 09-Dec-2018
 *      Author: hardi
 */

#include "led.h"

void LEDS_modeset(void)
{
	CMU_ClockEnable(cmuClock_GPIO, true);

	// configure LED pins
	GPIO_PinModeSet(BSP_LED0_PORT, BSP_LED0_PIN, gpioModePushPull,
	LED_OFF);
	GPIO_PinModeSet(BSP_LED1_PORT, BSP_LED1_PIN, gpioModePushPull,
	LED_OFF);

}

void LEDS_onoff(int state)
{
	switch (state)
	{
	case LED_ON:
		GPIO_PinOutSet(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutSet(BSP_LED1_PORT, BSP_LED1_PIN);
		break;

	case LED_OFF:
		GPIO_PinOutClear(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutClear(BSP_LED1_PORT, BSP_LED1_PIN);
		break;
	case LED_PROV:
		GPIO_PinOutToggle(BSP_LED0_PORT, BSP_LED0_PIN);
		GPIO_PinOutToggle(BSP_LED1_PORT, BSP_LED1_PIN);
		break;

	default:
		break;
	}
}


