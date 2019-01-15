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
#include "lcd_driver.h"
#include "retargetserial.h"
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
#define LOWEST_ENERGY_MODE 						(EM4)
#define TIME_PERIOD 							(3.00)
#define LED_ON_TIME 							(0.175)
#define TEMPERATURE_BUFFER_SIZE					(5)

/* Gecko configuration parameters*/

#define MAX_CONNECTIONS 4
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

//***********************************************************************************
// global variables
//***********************************************************************************

//Event Flag
//volatile uint8_t scheduled_event = 0;

int8_t rssi_value;
uint8_t connection_handle = 0;

//Bluetooth device address
struct gecko_msg_system_get_bt_address_rsp_t* bt_address_response;
char bt_address[BUFFER_SIZE];

//Client address
struct gecko_msg_le_connection_opened_evt_t client_address_response;
char string_c[BUFFER_SIZE];
char client_address[BUFFER_SIZE];

//Bonding failed reason
uint16_t reason_int;
char reason_buffer[BUFFER_SIZE];

//PAsskey
uint32_t passkey_int;
char passkey_buffer[BUFFER_SIZE];
char passkey[BUFFER_SIZE];

bool bonding_completed_flag = false;
bool passkey_confirmed_flag = false;

//***********************************************************************************
// Temperature Conversion Variables
//***********************************************************************************
// Temp data
extern uint16_t Rx_temp_code;
double DC_temperature = 0;

//***********************************************************************************
// function prototypes
//***********************************************************************************
void disable_pb0_ext_interrupt();
//void enable_pb0_ext_interrupt();
//***********************************************************************************
// functions
//***********************************************************************************

/**
 * @brief  This function diables button PB0 externa interrupt functionality
 * @param  void
 * @retval void
 */
void disable_pb0_ext_interrupt()
{
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeDisabled, 0);

	NVIC_DisableIRQ(GPIO_EVEN_IRQn);

}
/**
 * @brief  This function sets the new global transmission power value based
 * on the value of RSSI
 * @param  rssi_value: current RSSI value of the BLE
 * @retval no return value
 */

void set_tx_power(int8_t rssi_value)
{
	if (rssi_value > -35)
	{
		gecko_cmd_system_set_tx_power(-260);
	}
	else if (rssi_value < -35 && rssi_value > -45)
	{
		gecko_cmd_system_set_tx_power(-200);
	}
	else if (rssi_value < -45 && rssi_value > -55)
	{
		gecko_cmd_system_set_tx_power(-150);
	}
	else if (rssi_value < -55 && rssi_value > -65)
	{
		gecko_cmd_system_set_tx_power(50);
	}
	else if (rssi_value < -65 && rssi_value > -75)
	{
		gecko_cmd_system_set_tx_power(0);
	}
	else if (rssi_value < -75 && rssi_value > -85)
	{
		gecko_cmd_system_set_tx_power(50);
	}
	else
	{
		gecko_cmd_system_set_tx_power(80);
	}
}

/**
 * @brief  This function converts the degree celsius to Bluetooth SIG standard
 * bit stream format and sends it to the device
 * @param  DC_temp : temperature received from si7021 in degree celsius unit
 * @retval no return value
 *
 * @credit For calculating the temperature into machine readble format, the method used here is
 * from the soc-thermometer example from SiLabs with modifications in the calculation as the original method is not
 * giving desired result.
 */

void send_temperature_to_phone(double DC_temp)
{
	uint8_t temp_buffer[TEMPERATURE_BUFFER_SIZE];
	uint8_t flags = 0x00;
	uint32_t temp_to_send = 0;
	//uint32_t integer_value = 0;
	//double decimal_value = 0;
	uint8_t* ptrToBuffer = temp_buffer;
	char dc_temp[BUFFER_SIZE];

	//convert the temp to int value
	UINT8_TO_BITSTREAM(ptrToBuffer, flags);
	temp_to_send = FLT_TO_UINT32((DC_temp * 100), -2);
	UINT32_TO_BITSTREAM(ptrToBuffer, temp_to_send);

	//send the characteristic to the mobile app
	gecko_cmd_gatt_server_send_characteristic_notification(0xff,
	gattdb_temperature_measurement, 5, (uint8_t*) temp_buffer);

	//integer_value = DC_temp;
	//decimal_value = DC_temp - integer_value;

	sprintf(dc_temp, "Temp: %d.%d", (int) DC_temp,
			((int) (DC_temp * 100)) % 100);
	LCD_write(dc_temp, LCD_ROW_TEMPVALUE);
}
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

	//Initialize GPIO
	gpio_init();

	//letimer0 Setup
	setup_letimer0(LOWEST_ENERGY_MODE - 1, LED_ON_TIME, TIME_PERIOD);

	//timer setup
	setup_timer0();

	//i2c0 setup
	setup_I2C0();

	//Call this function to block the gecko below a particular energy mode
	//block_sleep_mode(LOWEST_ENERGY_MODE);

	//Enable letimer0
	LETIMER_Enable(LETIMER0, true);
	while (LETIMER0->SYNCBUSY)
		;

	// Initialize stack
	gecko_init(&config);

	RETARGET_SerialInit();
	RETARGET_SerialCrLf(true);

	LCD_init("BLE SERVER");

	//Start sleep routine to enter in the lowest possible energy mode
	while (1)
	{
		/* Event pointer for handling events */
		struct gecko_cmd_packet* evt;

		/* Check for stack event. */
		evt = gecko_wait_event();

		/* Handle events */
		switch (BGLIB_MSG_ID(evt->header))
		{
		/* This boot event is generated when the system boots up after reset.
		 * Do not call any stack commands before receiving the boot event.
		 * Here the system is set to start advertising immediately after boot procedure. */

		case gecko_evt_system_boot_id:

			//Set Transmission Power to 0dB after device is reset or disconnected
			gecko_cmd_system_halt(1);
			gecko_cmd_system_set_tx_power(0);
			gecko_cmd_system_halt(0);

			LCD_write("BT ADDRESS", LCD_ROW_BTADDR1);

			bt_address_response = gecko_cmd_system_get_bt_address();
			snprintf(bt_address, BUFFER_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X",
					bt_address_response->address.addr[5],
					bt_address_response->address.addr[4],
					bt_address_response->address.addr[3],
					bt_address_response->address.addr[2],
					bt_address_response->address.addr[1],
					bt_address_response->address.addr[0]);
			LCD_write(bt_address, LCD_ROW_BTADDR2);

			//LCD_write("00:0B:57:B5:F2:16", LCD_ROW_BTADDR2);

			/*Delete all prvious bondings*/
			gecko_cmd_sm_delete_bondings();

			/*Configure security requirements*/
			gecko_cmd_sm_configure(0x0F, sm_io_capability_displayyesno);

			/*allow bonding for new connections*/
			gecko_cmd_sm_set_bondable_mode(1);

			/* Set advertising parameters. 250ms advertisement interval.
			 * The first parameter is advertising set handle
			 * The next two parameters are minimum and maximum advertising interval, both in
			 * units of (milliseconds * 1.6).
			 * The last two parameters are duration and maxevents left as default. */
			gecko_cmd_le_gap_set_advertise_timing(0, BLE_MIN_ADVRT_TIME,
			BLE_MAX_ADVRT_TIME, 0, 0);

			/* Start general advertising and enable connections. */
			gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable,
					le_gap_connectable_scannable);
			break;

		case gecko_evt_le_connection_parameters_id:

			if (evt->data.evt_le_connection_parameters.security_mode
					>= le_connection_mode1_level3)
			{
				bonding_completed_flag = true;
				LCD_write("BONDED", LCD_ROW_CONNECTION);
			}

			break;

		case gecko_evt_le_connection_opened_id:

			LCD_write("CONNECTED", LCD_ROW_CONNECTION);

			//Enhance the current security level
			gecko_cmd_sm_increase_security(
					evt->data.evt_le_connection_opened.connection);

			/* Set Conncection parameters.
			 * The first parameter is connection handle
			 * The next two parameters are minimum and maximum conncection interval, both in
			 * uints of (milliseconds / 1.25)
			 * The third parameter is Slave letency
			 * The last parameter is left as default
			 * */

			gecko_cmd_le_connection_set_parameters(
					evt->data.evt_le_connection_opened.connection,
					BLE_MIN_CONNE_INTERVAL, BLE_MAX_CONNE_INTERVAL,
					BLE_SLAVE_LATENCY, BLE_SUPERVISON_TIME);

			snprintf(client_address, BUFFER_SIZE, "Client: %02x:%02x",
					evt->data.evt_le_connection_opened.address.addr[1],
					evt->data.evt_le_connection_opened.address.addr[0]);
			LCD_write(client_address, LCD_ROW_CLIENTADDR);

			break;

		case gecko_evt_sm_confirm_passkey_id:

			//display passkey to LCD
			connection_handle = evt->data.evt_sm_confirm_passkey.connection;
			passkey_int = evt->data.evt_sm_confirm_passkey.passkey;
			snprintf(passkey, BUFFER_SIZE, "Key : %ld", passkey_int);
			LCD_write(passkey, LCD_ROW_PASSKEY);

			LCD_write("Press PB0 to Confirm", LCD_ROW_ACTION);

			//set passkey confirmation flag to indicate passkey is confirmed by user
			passkey_confirmed_flag = true;

			break;

		case gecko_evt_sm_bonded_id:

			LCD_write("BONDED", LCD_ROW_CONNECTION);
			LCD_write("", LCD_ROW_ACTION);
			LCD_write("", LCD_ROW_PASSKEY);
			bonding_completed_flag = true;

			//diable PB0 button interrupt
			disable_pb0_ext_interrupt();

			break;

		case gecko_evt_sm_bonding_failed_id:

			LCD_write("BONDING FAILED", LCD_ROW_CONNECTION);
			reason_int = evt->data.evt_sm_bonding_failed.reason;
			snprintf(reason_buffer, BUFFER_SIZE, "Failed: %x", reason_int);
			LCD_write(reason_buffer, LCD_ROW_CONNECTION);

			passkey_confirmed_flag = false;
			bonding_completed_flag = false;

			break;

		case gecko_evt_gatt_server_characteristic_status_id:

			if (evt->data.evt_gatt_server_characteristic_status.status_flags
					== gatt_server_confirmation)
			{
				gecko_cmd_le_connection_get_rssi(
						evt->data.evt_gatt_server_characteristic_status.connection);
			}

			break;

		case gecko_evt_le_connection_rssi_id:

			//receive the current RSSI value
			rssi_value = evt->data.evt_le_connection_rssi.rssi;

			//halt the system
			gecko_cmd_system_halt(1);

			//set transmission power
			set_tx_power(rssi_value);

			//resume the system
			gecko_cmd_system_halt(0);

			break;

		case gecko_evt_system_external_signal_id:

			if (evt->data.evt_system_external_signal.extsignals
					& LETIMER0_EVENT_FLAG)
			{
				//scheduled_event &= ~LETIMER0_EVENT_FLAG;
				//Switch on Load Power Management
				load_power_management_on();
			}

			if (evt->data.evt_system_external_signal.extsignals
					& TIMER0_EVENT_FLAG)
			{

				//scheduled_event &= ~TIMER0_EVENT_FLAG;
				//Receive data from the sensor
				I2C_master_receiver();

				//Convert the temperature in degree celsius and work on led1
				//Rx_temperature_code = Rx_temp_code;
				DC_temperature = calculate_temp_celsius(Rx_temp_code);
				led_logic(DC_temperature);

				//if bonding is completed then show temprature to phone and display
				if (bonding_completed_flag)
				{
					send_temperature_to_phone(DC_temperature);
				}

				//turn of sensor and sda scl
				load_power_management_off();

				SLEEP_SleepBlockEnd(sleepEM2);

				//LETIMER enable
				LETIMER_Enable(LETIMER0, true);
			}

			if ((evt->data.evt_system_external_signal.extsignals
					& GPIO_PB0_EVENT_FLAG) && passkey_confirmed_flag)
			{
				//scheduled_event &= ~GPIO_PB0_EVENT_FLAG;
				LCD_write("PB0 Pressed", LCD_ROW_ACTION);
				gecko_cmd_sm_passkey_confirm(connection_handle, 1);
			}

			break;

		case gecko_evt_le_connection_closed_id:

			/* Check if need to boot to dfu mode */
			if (boot_to_dfu)
			{
				/* Enter to DFU OTA mode */
				gecko_cmd_system_reset(2);
			}
			else
			{
				LCD_write("DISCONNECTED", LCD_ROW_CONNECTION);
				LCD_write("", LCD_ROW_CLIENTADDR);
				LCD_write("", LCD_ROW_TEMPVALUE);

				bonding_completed_flag = false;
				passkey_confirmed_flag = false;

				//Enable Pb0 external interrupt again
				NVIC_EnableIRQ(GPIO_EVEN_IRQn);
				GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInput, 0);
				GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_INT_NUMBER, true,
				false, true);

				/* Restart advertising after client has disconnected */
				gecko_cmd_le_gap_start_advertising(0,
						le_gap_general_discoverable,
						le_gap_connectable_scannable);
			}
			break;

		default:
			break;

		}
	}

	return 0;
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
