/*
 * i2c0.h
 *
 *  Created on: 18-Sep-2018
 *      Author: hardi
 */

#ifndef I2C0_H_
#define I2C0_H_

#include "stdbool.h"
#include "em_i2c.h"
#include "gpio.h"
#include "sleep_routine.h"
#include "em_letimer.h"
//#include "load_power_management.h"

#define SENSOR_ADDRESS 						(0x40 << 1)
#define MEASUREMENT_COMMAND					(0xE3)
#define READ_FROM_SLAVE 					(0x01)
#define WRITE_TO_SLAVE						(0x00)

//Enable value for SCL and SDA in route enable register
#define I2C0_SCL_ROUTE_ENABLE				(0B000001)
#define I2C0_SDA_ROUTE_ENABLE				(0B000001)

//SCL and SDA pin location EXP 15 and 16
#define I2C0_SDA_PIN_LOCATION				(0B010000)
#define I2C0_SCL_PIN_LOCATION				(0B001110)

I2C_Init_TypeDef i2c0_init;

void setup_I2C0();
void route_i2c0_pins(void);
void set_i2c0_init_params(I2C_Init_TypeDef*);
void reset_i2c0_bus();
void reset_slave();
void i2c0_interrupt_setup();
//void I2C0_IRQHandler();

#endif /* I2C0_H_ */
