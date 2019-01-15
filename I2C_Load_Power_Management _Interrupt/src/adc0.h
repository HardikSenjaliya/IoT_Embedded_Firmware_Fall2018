/*
 * moisture_adc.h
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 */

#ifndef SOURCE_MOISTURE_ADC_H_
#define SOURCE_MOISTURE_ADC_H_

#include "stdbool.h"
#include "em_adc.h"
#include "em_cmu.h"
#include "em_core.h"
#include "native_gecko.h"
#include "stdio.h"

void soil_moisture_adc0_setup(void);
void soil_moisture_adc0_init(void);
void soil_moisture_adc0_start(void);


#endif /* SOURCE_MOISTURE_ADC_H_ */
