/*
 * load_power_management.c
 *
 *  Created on: 20-Sep-2018
 *      Author: hardi
 */

#include "load_power_management.h"

extern uint8_t scheduled_event;
/**
 * @brief  This function handles load power management.
 * It enables the temperature and humidity sensor and takes reading from it using
 * blue gecko in i2c master mode.
 * @param  no params
 * @retval no return value
 */

void load_power_management_on()
{
	//clear the event flag
	//scheduled_event = 0;

	//Enable the temperature and humidity sensor
	GPIO_PinOutSet(TEMP_HUMI_SENSOR_Port, SENSOR_ENABLE_PIN);

	//Set SCL line for I2C0
	GPIO_PinModeSet(I2C0_Port, I2C0_SCL_LINE, gpioModeWiredAnd, 1);
	//Set SDA line for I2C0
	GPIO_PinModeSet(I2C0_Port, I2C0_SDA_LINE, gpioModeWiredAnd, 1);

	//Reset the slave
	reset_slave();

	//block Em2 and below for timer0 andi2c0
	//block_sleep_mode(EM2);
	SLEEP_SleepBlockBegin(sleepEM2);

	//enable i2c
	I2C_Enable(I2C0, true);

	//reset i2c bus
	reset_i2c0_bus();

	//Wait while the sensor completes POR and/or SCL/SDA ramp up to high
	//call timer0 to give the wait time
	TIMER_Enable(TIMER0, true);
}

/**
 * @brief  This function converts temp code received from the sensor
 * into degree celsuis temperature it uses the equation from the sensor datasheet
 *  temp in degree celsius = {(temp code * 175.72)/ 65536} - 46.85
 * @param  temp code received from the sensor
 * @retval temperature reading in degree celsius
 */

double calculate_temp_celsius(uint16_t temp_code)
{
	double temp_in_c = 0;
	double temp = (temp_code * TEMP_MULTIPLIER) / TEMP_DIVISOR;
	temp_in_c = temp - TEMP_SUBTRACTOR;
	return temp_in_c;
}

/**
 * @brief  This function turns on led if temp is greater than 15 degree celsius
 * @param  temperature in degree celsius
 * @retval no return value
 */

void led_logic(float temp_in_c)
{
	//if temp is less than 15 turn led off else turn on
	if (temp_in_c < DEFAULT_TEMPERATURE)
	{
		GPIO_PinOutSet(LED1_port, LED1_pin);
	}
	else
	{
		GPIO_PinOutClear(LED1_port, LED1_pin);
	}
}

/**
 * @brief  This function manages load power management off situation
 * 		   it turns off the sensor and clear global flag to stop reding from sensor
 * @param  no params
 * @retval no return value
 */

void load_power_management_off()
{

	//Take SCL and SDA off the bus
	GPIO_PinModeSet(I2C0_Port, I2C0_SCL_LINE, gpioModeDisabled, 0);
	GPIO_PinModeSet(I2C0_Port, I2C0_SDA_LINE, gpioModeDisabled, 0);

	//Turn off the sensor enable pin
	//GPIO_PinOutClear(TEMP_HUMI_SENSOR_Port, SENSOR_ENABLE_PIN);
}
