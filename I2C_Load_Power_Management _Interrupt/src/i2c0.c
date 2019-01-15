/*
 * i2c0.c
 *
 *  Created on: 18-Sep-2018
 *      Author: hardi
 */
#include "i2c0.h"

//Buffer to store received data
static uint16_t Rx_Buffer[] =
{ 0, 0 };
uint16_t Rx_temp_code = 0;

//Global flag for event
extern uint8_t scheduled_event;

//Global flag for interrupts
volatile uint8_t ACK = 0;
volatile uint8_t RXDATAV = 0;

/**
 * @brief  setup i2c0
 * @retval no return value
 */

void setup_I2C0()
{
	//Disabled sensor pin
	GPIO_PinModeSet(TEMP_HUMI_SENSOR_Port, SENSOR_ENABLE_PIN, gpioModePushPull,
			0);
	GPIO_PinOutClear(TEMP_HUMI_SENSOR_Port, SENSOR_ENABLE_PIN);

	//Configure i2c0 parameters
	set_i2c0_init_params(&i2c0_init);

	//Route i2c pins to the i2c0 peripheral
	route_i2c0_pins();

	//Initialize i2c0
	I2C_Init(I2C0, &i2c0_init);

	//Enable I2C interrupt
	i2c0_interrupt_setup();

	NVIC_EnableIRQ(I2C0_IRQn);
}

/**
 * @brief  Route i2c pins to the location
 * @param  void
 * @retval no return value
 */

void route_i2c0_pins()
{
	//Enable SCL and SDA in the routine pin enable register
	I2C0->ROUTEPEN = I2C_ROUTEPEN_SCLPEN | I2C_ROUTEPEN_SDAPEN;

	//Route i2c0 SCL pin to EXP 15 and SDA to EXP 16
	I2C0->ROUTELOC0 = I2C_ROUTELOC0_SCLLOC_LOC14 | I2C_ROUTELOC0_SDALOC_LOC16;
}

/**
 * @brief  initialize i2c0 parameters
 * @param  i2c: pointer to structure I2C_Init_TypeDef
 * @retval no return value
 */

void set_i2c0_init_params(I2C_Init_TypeDef* i2c0_init)
{
	i2c0_init->enable = false;
	i2c0_init->master = true;
	i2c0_init->refFreq = 0;
	i2c0_init->freq = I2C_FREQ_STANDARD_MAX;
	i2c0_init->clhr = i2cClockHLRStandard;
}

/**
 * @brief  reset the i2c bus for sync purposes
 * @param  void
 * @retval no return value
 */

void reset_i2c0_bus()
{
	if (I2C0->STATE & I2C_STATE_BUSY)
	{
		I2C0->CMD = I2C_CMD_ABORT;
	}
}

/**
 * @brief  resets the slave
 * @param  void
 * @retval no return value
 */

void reset_slave()
{
	uint32_t index;

	//reset i2c slave
	for (index = 0; index < 9; ++index)
	{
		GPIO_PinOutClear(I2C0_Port, I2C0_SCL_LINE);
		GPIO_PinOutSet(I2C0_Port, I2C0_SCL_LINE);
	}
}

/**
 * @brief  This function is master i2c receiver driver. It takes reading from
 * the sensor
 * @param  no params
 * @retval no return value
 */

void I2C_master_receiver()
{
	scheduled_event = 0;
	LETIMER_Enable(LETIMER0, false);

	//Reset the slave
	reset_slave();

	//Abort current operation if i2c peripheral is busy and reset it
	reset_i2c0_bus();

	//clear buffers
	I2C0->CMD = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX;

	if (I2C0->IF & I2C_IF_RXDATAV)
	{
		(void) I2C0->RXDATA;
	}

	//Sending the start bit to the slave
	I2C0->CMD = I2C_CMD_START;

	//Send the slave address
	I2C0->TXDATA = SENSOR_ADDRESS | WRITE_TO_SLAVE;

	//wait for ack and do the next step
	while (!ACK)
	{
		//enter_sleep_routine();
	}

	ACK = 0;

	I2C0->TXDATA = MEASUREMENT_COMMAND;

	while (!ACK)
	{
		//enter_sleep_routine();
	}

	ACK = 0;

	//Send the START REPEAT command
	I2C0->CMD = I2C_CMD_START;
	//Send the device address to read from it
	I2C0->TXDATA = SENSOR_ADDRESS | READ_FROM_SLAVE;

	while (!ACK)
	{
		//enter_sleep_routine();
	}

	ACK = 0;

	while (!RXDATAV)
	{
		//enter_sleep_routine();
	}

	RXDATAV = 0;

	//Send ACK to the slave
	I2C0->CMD = I2C_CMD_ACK;

	while (!RXDATAV)
	{
		//enter_sleep_routine();
	}

	RXDATAV = 0;

	//Send NACK and STOP
	I2C0->CMD = I2C_CMD_NACK;
	I2C0->CMD = I2C_CMD_STOP;
	//set event complete flag

	RXDATAV = 0;

	//set the recevied temp code to the local bufffer
	Rx_temp_code = (Rx_Buffer[1]) | (Rx_Buffer[0] << 8);

}

/**
 * @brief  enable i2c interrupts
 * @param  void
 * @retval no return value
 */

void i2c0_interrupt_setup()
{
	//uint32_t pending_interrupts = I2C_IntGet(I2C0);
	I2C_IntClear(I2C0, 0xFFFF);

	//Enable all the required I2C0 interrupts and NVIC
	//Enable NACK interrupt
	I2C0->IEN |= I2C_IEN_NACK;

	//Enable ACK interrupt
	I2C0->IEN |= I2C_IEN_ACK;

	//Enable RXDATAV interrupt
	I2C0->IEN |= I2C_IEN_RXDATAV;
}

/**
 * @brief  interrupt handler for i2c0
 * @param  void
 * @retval no return value
 */

void I2C0_IRQHandler()
{
	/*	static uint16_t ACK_count = 0;
	 static uint16_t RXDATAV_count = 0;*/
	static int index = 0;

	CORE_ATOMIC_IRQ_DISABLE();
	uint32_t pending_intrreupt_flag = I2C_IntGet(I2C0);

	I2C_IntClear(I2C0, pending_intrreupt_flag);

	//Handle NACK interrupt
	if (pending_intrreupt_flag & I2C_IF_NACK)
	{

		//clear buffers
		I2C0->CMD = I2C_CMD_CLEARPC | I2C_CMD_CLEARTX;

		I2C0->CMD = I2C_CMD_ABORT;

		//Sending the start bit to the slave
		I2C0->CMD = I2C_CMD_START;

		//Send the slave address
		I2C0->TXDATA = SENSOR_ADDRESS | WRITE_TO_SLAVE;

		I2C_IntClear(I2C0, I2C_IFC_NACK);
	}

	//Handle ACK interrupt
	if (pending_intrreupt_flag & I2C_IF_ACK)
	{
		ACK = 1;
		I2C_IntClear(I2C0, I2C_IFC_ACK);
	}

	//check for the recevied data
	if (pending_intrreupt_flag & I2C_IF_RXDATAV)
	{
		RXDATAV = 1;
		Rx_Buffer[index++] = I2C0->RXDATA;
		//I2C_IntClear(I2C0, I2C_IF_RXDATAV);
		if (index > 1)
		{
			index = 0;
			scheduled_event = 3;
		}

	}

	CORE_ATOMIC_IRQ_ENABLE();
}
