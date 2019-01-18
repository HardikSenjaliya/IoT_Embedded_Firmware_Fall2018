/*
 * moisture_adc.h
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 */

#ifndef SOURCE_ADC0_H_
#define SOURCE_ADC0_H_

#include "stdbool.h"
#include "em_adc.h"
#include "em_cmu.h"
#include "em_core.h"
#include "native_gecko.h"
#include "stdio.h"
#include "display_interface.h"
#include "lcd_driver.h"


void soil_moisture_adc0_setup(void);
void soil_moisture_adc0_init(void);
uint32_t get_soil_moisture_calibrated_value(uint32_t moisture_level);
uint16_t moisture_to_onoff_commnad(uint32_t moisture_level);
void adc0_clock_setup(void);

#endif /* SOURCE_ADC0_H_ */
