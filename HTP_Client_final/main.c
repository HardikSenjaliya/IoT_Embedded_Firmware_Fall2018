/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Thermometer Example Application
 *
 * This Thermometer and OTA example allows the user to measure temperature
 * using the temperature sensor on the WSTK. The values can be read with the
 * Health Thermometer reader on the Blue Gecko smartphone app.
 ***************************************************************************************************
 * <b> (C) Copyright 2016 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silicon Labs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* Board Headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "lcd_driver.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "infrastructure.h"

/* GATT database */
#include "gatt_db.h"

/* EM library (EMlib) */
#include "em_system.h"

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include "gpiointerrupt.h"
#include "retargetserial.h"
#include "src/main.h"

/* Device initialization header */
#include "hal-config.h"

#ifdef FEATURE_BOARD_DETECTED
#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#else
#include "bspconfig.h"
#endif
#else
#error This sample app only works with a Silicon Labs Board
#endif

#include "i2cspm.h"
#include "si7013.h"
#include "tempsens.h"

/***********************************************************************************************//**
 * @addtogroup Application
 * @{
 **************************************************************************************************/

/***********************************************************************************************//**
 * @addtogroup app
 * @{
 **************************************************************************************************/

//***********************************************************************************
// function prototypes
//***********************************************************************************
/* Gecko configuration parameters (see gecko_configuration.h) */
#ifndef MAX_CONNECTIONS
#define MAX_CONNECTIONS 			(4)
#endif

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)];

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

/* Flag for indicating DFU Reset must be performed */
uint8_t boot_to_dfu = 0;

/*Server address*/
bd_addr server_address =
{ .addr =
{ 0x71, 0xF2, 0xB5, 0x57, 0x0B, 0x00 } };

//Bluetooth device address
struct gecko_msg_system_get_bt_address_rsp_t* bt_address_response;
char bt_address[BUFFER_SIZE];

//Client address
struct gecko_msg_le_connection_opened_evt_t client_address_response;
char client_address[BUFFER_SIZE];

//PAsskey
uint32_t passkey_int;
char passkey_buffer[BUFFER_SIZE];
char passkey[BUFFER_SIZE];

//Bonding failed reason
uint16_t reason_int;
char reason_buffer[BUFFER_SIZE];

uint8_t connection_handle;
uint32_t HTM_service_handle;
uint16_t temp_cal_characteristic_handle;

int current_status = DISCONNECTED;
uint8_t temp_data_from_server[10] =
{ "0" };
double temp_in_celsius = 0;

bool bonding_completed_flag = false;
bool passkey_confirmed_flag = false;

/**
 * @brief  This function is a callback function of the button PB0 interrupt
 * whenever interrupts occours it sends and external BLE event.
 * @param  Pin: button Pb0 pin number
 * @retval void
 */

void pb0_interrupt_handler(uint8_t pin)
{
	if (pin == PB0_pin)
	{
		//scheduled_event |= GPIO_PB0_EVENT_FLAG;
		gecko_external_signal(GPIO_PB0_EVENT_FLAG);
	}
}

void disable_pb0_ext_interrupt()
{
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeDisabled, 0);

	NVIC_DisableIRQ(GPIO_EVEN_IRQn);
}

void enable_pb0_ext_interrupt()
{
	//Set PB0 as an external interrupt source
	GPIOINT_Init();
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInput, 0);
	GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInput, 0);
	GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_INT_NUMBER, true, false, true);
	GPIOINT_CallbackRegister(PB0_pin, pb0_interrupt_handler);

	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
}
/**
 * @brief  Main function
 */
int main(void)
{
	// Initialize device
	initMcu();
	// Initialize board
	initBoard();
	// Initialize application
	initApp();

	// Initialize stack
	gecko_init(&config);

	RETARGET_SerialInit();
	RETARGET_SerialCrLf(true);

	LCD_init("BLE Client");

	/*service uuid*/
	uint8array service_uuid;
	service_uuid.data[0] = 0x09;
	service_uuid.data[1] = 0x18;
	service_uuid.len = 2;

	/*characteristic uuid*/
	uint8array characteristic_uuid;
	characteristic_uuid.data[0] = 0x1C;
	characteristic_uuid.data[1] = 0x2A;
	characteristic_uuid.len = 2;

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

			//gecko_cmd_sm_delete_bondings();

			bt_address_response = gecko_cmd_system_get_bt_address();
			snprintf(bt_address, BUFFER_SIZE, "%02X:%02X:%02X:%02X:%02X:%02X",
					bt_address_response->address.addr[5],
					bt_address_response->address.addr[4],
					bt_address_response->address.addr[3],
					bt_address_response->address.addr[2],
					bt_address_response->address.addr[1],
					bt_address_response->address.addr[0]);

			printf("Device Address:  %s\n", bt_address);
			LCD_write("BT ADDRESS", LCD_ROW_BTADDR1);
			LCD_write(bt_address, LCD_ROW_BTADDR2);

			/*Check if pb1 is pressed or not
			 * if pressed then delete all the
			 * previous bondings*/
			if (GPIO_PinInGet(PB1_port, PB1_pin))
			{
				//delete all bondings to force the pairing process
				LCD_write("PB1 Pressed", LCD_ROW_ACTION);
				gecko_cmd_sm_delete_bondings();
				printf("Delete all bondings\n");
			}

			gecko_cmd_sm_set_bondable_mode(1);

			gecko_cmd_sm_configure(0x0F, sm_io_capability_displayyesno);

			/*Send commnad to connect the server*/
			struct gecko_msg_le_gap_connect_rsp_t* gap_connect_response =
					gecko_cmd_le_gap_connect(server_address,
							le_gap_identity_address, le_gap_phy_1m);

			if (gap_connect_response->result != 0)
			{
				reason_int = gap_connect_response->result;
				snprintf(reason_buffer, BUFFER_SIZE, "Reason: %x", reason_int);
				LCD_write(reason_buffer, LCD_ROW_CONNECTION);

				printf("Connecting failed with reason: %x!!\n", reason_int);
			}

			break;

		case gecko_evt_le_connection_opened_id:

			/*connection handle from the opened connection*/
			connection_handle = evt->data.evt_le_connection_opened.connection;

			printf("CONNECTED\n");
			LCD_write("CONNECTED", LCD_ROW_CONNECTION);

			snprintf(client_address, BUFFER_SIZE, "Sever: %02x:%02x",
					evt->data.evt_le_connection_opened.address.addr[1],
					evt->data.evt_le_connection_opened.address.addr[0]);
			LCD_write(client_address, LCD_ROW_CLIENTADDR);

			printf("Server: %02x:%02x\n",
					evt->data.evt_le_connection_opened.address.addr[1],
					evt->data.evt_le_connection_opened.address.addr[0]);

			/*check if devices are already bonded or not
			 *if already bonded then start discovering services*/
			if (evt->data.evt_le_connection_opened.bonding != 0xFF)
			{
				LCD_write("ALREADY BONDED", LCD_ROW_CONNECTION);
				printf("ALREADY BONDED\n");
				LCD_write("", LCD_ROW_ACTION);
				LCD_write("", LCD_ROW_PASSKEY);

				/*diable PB0 button interrupt*/
				disable_pb0_ext_interrupt();

				/*discover Heatlh Thermometer Service using UUID*/
				printf("Discovering primary services\n");
				struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t* discover_service_response;
				discover_service_response =
						gecko_cmd_gatt_discover_primary_services_by_uuid(
								evt->data.evt_le_connection_opened.connection, service_uuid.len,
								service_uuid.data);

				/*if there is an error in finding the service
				 *print the error number and close the connection*/
				if (discover_service_response->result != 0)
				{
					printf("Discovering services failed\n");
					reason_int = discover_service_response->result;
					snprintf(reason_buffer, BUFFER_SIZE, "Reason: %x",
							reason_int);
					LCD_write(reason_buffer, LCD_ROW_CONNECTION);

					/*close the connection*/
					gecko_cmd_le_connection_close(connection_handle);
				}
			}

			break;

		case gecko_evt_sm_confirm_bonding_id:

			/*accept the bonding request*/
			gecko_cmd_sm_bonding_confirm(
					evt->data.evt_sm_confirm_bonding.connection, 1);

			break;

		case gecko_evt_sm_confirm_passkey_id:

			/*display passkey on LCD*/
			passkey_int = evt->data.evt_sm_confirm_passkey.passkey;
			snprintf(passkey, BUFFER_SIZE, "Key : %ld", passkey_int);
			LCD_write(passkey, LCD_ROW_PASSKEY);

			printf("Passkey: %lu\n", passkey_int);

			/*Enable PB0 button interrupt for taking
			 * confirmation from the user*/
			enable_pb0_ext_interrupt();

			LCD_write("Press PB0 to Confirm", LCD_ROW_ACTION);
			printf("Press PB0 to confirm\n");

			/*set passkey confirmation flag to indicate passkey is confirmed by user*/
			passkey_confirmed_flag = true;

			break;

		case gecko_evt_sm_bonded_id:

			printf("BONDED\n");
			LCD_write("BONDED", LCD_ROW_CONNECTION);
			LCD_write("", LCD_ROW_ACTION);
			LCD_write("", LCD_ROW_PASSKEY);
			bonding_completed_flag = true;

			/*disable PB0 button interrupt after bonding*/
			disable_pb0_ext_interrupt();

			/*Now devices are bonded hence search for primary services*/
			current_status = DISCOVER_SERVICE;
			struct gecko_msg_gatt_discover_primary_services_by_uuid_rsp_t* discover_service_response;
			discover_service_response =
					gecko_cmd_gatt_discover_primary_services_by_uuid(
							evt->data.evt_sm_bonded.connection,
							service_uuid.len, service_uuid.data);

			/*If there is an error in finding the service
			 * close the connection*/
			if (discover_service_response->result != 0)
			{
				printf("Discovering services failed\n");

				reason_int = discover_service_response->result;
				snprintf(reason_buffer, BUFFER_SIZE, "Reason: %x", reason_int);
				LCD_write(reason_buffer, LCD_ROW_CONNECTION);

				/*Close the connection*/
				gecko_cmd_le_connection_close(evt->data.evt_sm_bonded.connection);
			}

			break;

		case gecko_evt_sm_bonding_failed_id:

			/*If bonding is failed display the Reason on LCD
			 * and close the connection*/
			printf("BONDING FAILED with reason: %x\n", reason_int);
			LCD_write("BONDING FAILED", LCD_ROW_CONNECTION);
			reason_int = evt->data.evt_sm_bonding_failed.reason;
			snprintf(reason_buffer, BUFFER_SIZE, "Failed: %x", reason_int);
			LCD_write(reason_buffer, LCD_ROW_CONNECTION);

			passkey_confirmed_flag = false;

			/*Close the connection*/
			gecko_cmd_le_connection_close(connection_handle);
			break;

		case gecko_evt_gatt_service_id:

			printf("Service event\n");

			/*check for required service is found or not
			 * if found store the service handle and start discovering
			 * characteristics */

			if (!(memcmp(&(evt->data.evt_gatt_service.uuid.data[0]),
					&service_uuid.data[0], service_uuid.len)))
			{
				printf("Service found\n");
				current_status = SERVICE_FOUND;
				/*Store the service handle*/
				HTM_service_handle = evt->data.evt_gatt_service.service;
			}

			break;

		case gecko_evt_gatt_characteristic_id:

			printf("Characteristic event\n");

			/*check for required characteristic is found or not
			 * if found set characteristic notification*/
			if (!(memcmp(&(evt->data.evt_gatt_characteristic.uuid.data[0]),
					&characteristic_uuid.data[0], characteristic_uuid.len)))
			{
				printf("Characteristic found\n");
				current_status = CHARACTERISTIC_FOUND;
				temp_cal_characteristic_handle =
						evt->data.evt_gatt_characteristic.characteristic;
			}

			break;

		case gecko_evt_gatt_characteristic_value_id:

			printf("Characteristic value event\n");

			/*check the value is received from the desired characteristic or not*/
			if (evt->data.evt_gatt_characteristic_value.characteristic
					== temp_cal_characteristic_handle)
			{
				/*Check for which event causes this event and if it is Indication from the
				 * server then send the confirmation*/
				if (evt->data.evt_gatt_characteristic_value.att_opcode == 0x1D)
				{
					/*send confirmation to the server*/
					struct gecko_msg_gatt_send_characteristic_confirmation_rsp_t* confirmation_response =
							gecko_cmd_gatt_send_characteristic_confirmation(
									connection_handle);

					if (confirmation_response->result != 0)
					{
						printf(
								"Error in sending characteristic confirmation\n");
					}
				}

				/*Send the received data to LCD*/
				memcpy(temp_data_from_server,
						&(evt->data.evt_gatt_characteristic_value.value.data[0]),
						10);

				temp_in_celsius = UINT32_TO_FLT(
						*(uint32_t* )&temp_data_from_server[1]);

				char temp_buffer[BUFFER_SIZE] =
				{ "0" };

				sprintf(temp_buffer, "Temp: %d.%d", (int) temp_in_celsius,
						((int) (temp_in_celsius * 100)) % 100);

				LCD_write(temp_buffer, LCD_ROW_TEMPVALUE);
			}

			break;

		case gecko_evt_gatt_procedure_completed_id:

			printf("procedure completed event\n");

			switch (current_status)
			{

			/*service is found hence start discovering required characteristic*/
			case SERVICE_FOUND:

				printf("Service found case\n");

				current_status = DISCOVER_CHARACTERISTIC;

				/*discover characteristic using uuid*/
				struct gecko_msg_gatt_discover_characteristics_by_uuid_rsp_t* discover_char_response =
						gecko_cmd_gatt_discover_characteristics_by_uuid(
								connection_handle, HTM_service_handle,
								characteristic_uuid.len,
								characteristic_uuid.data);

				/*if error in discovering the characteristic
				 * close the connection and display the reason on LCD*/
				if (discover_char_response->result != 0)
				{
					printf("Error in finding characteristics\n");

					reason_int = discover_char_response->result;
					snprintf(reason_buffer, BUFFER_SIZE, "Reason: %d",
							reason_int);
					LCD_write(reason_buffer, LCD_ROW_CONNECTION);

					/*close the connection*/
					gecko_cmd_le_connection_close(connection_handle);
				}

				break;

				//characteristic is found hence set the notification for the characteristic
			case CHARACTERISTIC_FOUND:

				printf("Characteristic found case\n");
				current_status = ENABLE_NOTIFICATION;

				/*set indication for the characteristic*/
				struct gecko_msg_gatt_set_characteristic_notification_rsp_t* notification_response

				= gecko_cmd_gatt_set_characteristic_notification(
						connection_handle, temp_cal_characteristic_handle,
						gatt_indication);

				/*if error in setting the indication, display
				 * the reason and close the connection*/
				if (notification_response->result != 0)
				{
					printf("Error in setting notification\n");

					reason_int = notification_response->result;
					snprintf(reason_buffer, BUFFER_SIZE, "Reason: %d",
							reason_int);
					LCD_write(reason_buffer, LCD_ROW_CONNECTION);

					gecko_cmd_le_connection_close(connection_handle);
				}

				break;

			case ENABLE_NOTIFICATION:

				printf("enable notification case\n");

				if (evt->data.evt_gatt_procedure_completed.result != 0)
				{
					/*nable to set notification close the connection*/
					gecko_cmd_le_connection_close(connection_handle);
				}

				break;

			default:
				printf("Deafault case \n");
				break;
			}

			break;

		case gecko_evt_system_external_signal_id:

			printf("external signal event");
			/*check the event is Button pressed or not
			 * if yes then send the passkey confirmation*/
			if ((evt->data.evt_system_external_signal.extsignals
					& GPIO_PB0_EVENT_FLAG) && passkey_confirmed_flag)
			{
				LCD_write("PB0 Pressed", LCD_ROW_ACTION);
				printf("PB0 Pressed\n");

				/*sending passkey confirmation*/
				gecko_cmd_sm_passkey_confirm(connection_handle, 1);
			}

			break;

		case gecko_evt_le_connection_closed_id:

			printf("conncection closed event\n");
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

				printf("DISCONNECTED");
				passkey_confirmed_flag = false;

				/*Enable PB0 button interrupt again*/
				enable_pb0_ext_interrupt();
			}
			break;

			/* Events related to OTA upgrading
			 ----------------------------------------------------------------------------- */

			/* Checks if the user-type OTA Control Characteristic was written.
			 * If written, boots the device into Device Firmware Upgrade (DFU) mode. */
		case gecko_evt_gatt_server_user_write_request_id:
			if (evt->data.evt_gatt_server_user_write_request.characteristic
					== gattdb_ota_control)
			{
				/* Set flag to enter to OTA mode */
				boot_to_dfu = 1;
				/* Send response to Write Request */
				gecko_cmd_gatt_server_send_user_write_response(
						evt->data.evt_gatt_server_user_write_request.connection,
						gattdb_ota_control, bg_err_success);

				/* Close connection to enter to DFU OTA mode */
				gecko_cmd_le_connection_close(
						evt->data.evt_gatt_server_user_write_request.connection);
			}
			break;

		default:
			break;
		}
	}
}

/** @} (end addtogroup app) */
/** @} (end addtogroup Application) */
