/*
 * load_power_management.h
 *
 *  Created on: 20-Sep-2018
 *      Author: hardi
 */

#ifndef LOAD_POWER_MANAGEMENT_H_
#define LOAD_POWER_MANAGEMENT_H_

#include "gpio.h"
#include "i2c0.h"
#include "timer.h"
#include "sleep_routine.h"
#include "letimer0.h"

#define DEFAULT_TEMPERATURE					(15.0f)
#define TEMP_MULTIPLIER						(175.72)
#define TEMP_DIVISOR						(65536)
#define TEMP_SUBTRACTOR						(46.85)

extern uint32_t timer0_int_flag;

void load_power_management_on();
void I2C_master_receiver();
double calculate_temp_celsius(uint16_t);
void led_logic(float);
void load_power_management_off();

#endif /* LOAD_POWER_MANAGEMENT_H_ */


