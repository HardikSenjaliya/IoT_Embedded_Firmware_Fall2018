/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Empty Example Project
 *
 * This example demonstrates the bare minimum needed for a Blue Gecko C application
 * that allows Over-the-Air Device Firmware Upgrading (OTA DFU). The application
 * starts advertising after boot and restarts advertising after a connection is closed.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "ble-configuration.h"
#include "board_features.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "em_letimer.h"

/* Device initialization header */
#include "hal-config.h"

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 4
#endif
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

// Gecko configuration parameters (see gecko_configuration.h)
static const gecko_configuration_t config =
{ .config_flags = 0, .sleep.flags = SLEEP_FLAGS_DEEP_SLEEP_ENABLE,
		.bluetooth.max_connections = MAX_CONNECTIONS, .bluetooth.heap =
				bluetooth_stack_heap, .bluetooth.heap_size =
				sizeof(bluetooth_stack_heap),
		.bluetooth.sleep_clock_accuracy = 100, // ppm
		.gattdb = &bg_gattdb_data, .ota.flags = 0, .ota.device_name_len = 3,
		.ota.device_name_ptr = "OTA",
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.pa.config_enable = 1, // Enable high power PA
		.pa.input = GECKO_RADIO_PA_INPUT_VBAT,// Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
	};

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;
//***********************************************************************************
// Include files
//***********************************************************************************

#include "src/main.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//Energy_Modes LOWEST_ENERGY_MODE = EM0;
// set LOWEST_ENERGY_MODE as per the requirement
// EM0 = 0, EM1 = 1, EM2 = 2, EM3 = 3
// Note: If you want to use EM0 mode set #define to EM1
#define LOWEST_ENERGY_MODE EM4
#define TIME_PERIOD 3.00
#define LED_ON_TIME 0.175

//***********************************************************************************
// global variables
//***********************************************************************************

//Event Flag
uint8_t scheduled_event = 0;

// Temp data
extern uint16_t Rx_temp_code;
uint16_t Rx_temperature_code = 0;
float DC_temperature = 0;
//***********************************************************************************
// function prototypes
//***********************************************************************************

//***********************************************************************************
// functions
//***********************************************************************************

//***********************************************************************************
// main
//***********************************************************************************

/**
 * @brief  Main function
 */
int main(void)
{
	//Initialize device
	initMcu();

	//Initialize board
	initBoard();

	//Initialize clocks
	cmu_init(LOWEST_ENERGY_MODE - 1);

	//Initialize stack
	gecko_init(&config);

	//Initialize GPIO
	gpio_init();

	soil_moisture_adc0_setup();

	//letimer0 Setup
	//setup_letimer0(LOWEST_ENERGY_MODE - 1, LED_ON_TIME, TIME_PERIOD);

	//timer setup
	//setup_timer0();

	//i2c0 setup
	//setup_I2C0();

	//Call this function to block the gecko below a particular energy mode
	//block_sleep_mode(LOWEST_ENERGY_MODE);

	//Enable letimer0
	//LETIMER_Enable(LETIMER0, true);
	//while (LETIMER0->SYNCBUSY);

	//Start sleep routine to enter in the lowest possible energy mode
	while (1)
	{

		/*if (!scheduled_event)
		{
			enter_sleep_routine();
		}

		if (scheduled_event == 1)
		{
			//Switch on Load Power Management
			load_power_management_on();

		}
		if (scheduled_event == 2)
		{
			//Receive data from the sensor
			I2C_master_receiver();

			//Convert the temperature in degree celsius and work on led1
			Rx_temperature_code = Rx_temp_code;
			DC_temperature = calculate_temp_celsius(Rx_temperature_code);
			led_logic(DC_temperature);

			//turn of sensor and sda scl
			load_power_management_off();

			//Unblock EM2 mode
			unblock_sleep_mode(EM2);

			//LETIMER
			LETIMER_Enable(LETIMER0, true);

			scheduled_event = 0;
		}
*/	}

}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
