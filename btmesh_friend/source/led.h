/*
 * led.h
 *
 *  Created on: 09-Dec-2018
 *      Author: hardi
 */

#ifndef SOURCE_LED_H_
#define SOURCE_LED_H_

#include "em_gpio.h"
#include "bsphalconfig.h"
#include "em_cmu.h"

#define LED_OFF							  	(0)
#define LED_ON							  	(1)
#define LED_PROV						  	(2)

void LEDS_onoff(int state);
void LEDS_modeset(void);


#endif /* SOURCE_LED_H_ */
