/***********************************************************************************************//**
 * \file   main.c
 * \brief  Silicon Labs Bluetooth mesh light example
 *
 * This example implements a Bluetooth mesh light node.
 *
 ***************************************************************************************************
 * <b> (C) Copyright 2017 Silicon Labs, http://www.silabs.com</b>
 ***************************************************************************************************
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 **************************************************************************************************/

/* C Standard Library headers */
#include <adc0.h>
#include <stdlib.h>
#include <stdio.h>

/* Board headers */
#include "init_mcu.h"
#include "init_board.h"
#include "init_app.h"
#include "ble-configuration.h"
#include "board_features.h"
#include "retargetserial.h"

/* Bluetooth stack headers */
#include "bg_types.h"
#include "native_gecko.h"
#include "gatt_db.h"
#include <gecko_configuration.h>
#include "mesh_generic_model_capi_types.h"
#include "mesh_lighting_model_capi_types.h"
#include "mesh_lib.h"
#include <mesh_sizes.h>

/* Libraries containing default Gecko configuration values */
#include "em_emu.h"
#include "em_cmu.h"
#include <em_gpio.h>
#include "em_rtcc.h"
#include "gpiointerrupt.h"
#include "adc0.h"
#include "led.h"
#include "letimer0.h"

/* Device initialization header */
#include "hal-config.h"

/* Display Interface header */
#include "display_interface.h"

/* LED driver with support for PWM dimming */
#include "led_driver.h"

/* ADC drvier*/

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

struct mesh_generic_state current, target;

bool mesh_bgapi_listener(struct gecko_cmd_packet *evt);

// Maximum number of simultaneous Bluetooth connections
#define MAX_CONNECTIONS 2

// heap for Bluetooth stack
uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(MAX_CONNECTIONS)
		+ BTMESH_HEAP_SIZE + 1760];

// Flag for indicating DFU Reset must be performed
uint8_t boot_to_dfu = 0;

// Bluetooth advertisement set configuration
//
// At minimum the following is required:
// * One advertisement set for Bluetooth LE stack (handle number 0)
// * One advertisement set for Mesh data (handle number 1)
// * One advertisement set for Mesh unprovisioned beacons (handle number 2)
// * One advertisement set for Mesh unprovisioned URI (handle number 3)
// * N advertisement sets for Mesh GATT service advertisements
// (one for each network key, handle numbers 4 .. N+3)
//
#define MAX_ADVERTISERS (4 + MESH_CFG_MAX_NETKEYS)

// Bluetooth stack configuration
const gecko_configuration_t config =
{ .bluetooth.max_connections = MAX_CONNECTIONS, .bluetooth.max_advertisers =
MAX_ADVERTISERS, .bluetooth.heap = bluetooth_stack_heap, .bluetooth.heap_size =
		sizeof(bluetooth_stack_heap) - BTMESH_HEAP_SIZE,
		.bluetooth.sleep_clock_accuracy = 100, .gattdb = &bg_gattdb_data,
		.btmesh_heap_size = BTMESH_HEAP_SIZE,
#if (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.pa.config_enable = 1, // Enable high power PA
		.pa.input = GECKO_RADIO_PA_INPUT_VBAT,// Configure PA input to VBAT
#endif // (HAL_PA_ENABLE) && defined(FEATURE_PA_HIGH_POWER)
		.max_timers = 16, };

/** Timer Frequency used. */
#define TIMER_CLK_FREQ ((uint32)32768)
/** Convert msec to timer ticks. */
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)

#define TIMER_ID_STOP_ADC_WHILE_RAINING		(52)
#define TIMER_ID_SOIL_MOISTURE_DATA 		(51)
#define TIMER_ID_RESTART    				(50)
#define TIMER_ID_FACTORY_RESET  			(49)
#define TIMER_ID_PROVISIONING   			(48)
#define TIMER_ID_SAVE_STATE   				(47)
#define TIMER_ID_DELAYED_PRI_LEVEL        	(44)
#define TIMER_ID_PRI_LEVEL_TRANSITION     	(43)
#define TIMER_ID_RETRANS				  	(42)
#define TIMER_ID_SEND_MOISTURE_DATA			(41)
#define TIMER_ID_SETUP_MOISTURE_SENSOR		(40)

#define LETIMER0_INTERRUPT_EVENT			(11)
#define ADC_SAMPLING_TIME					(15)
#define ADC_CMD_SINGLESTOP_MASK				(1)
#define ENERGY_MODE							(0)
#define SPRINKLER_STATE_SHIFT				(8)
#define SPRINKLER_STATE_ON					(1)
#define SPRINKLER_STATE_OFF					(0)
#define ADC_DATA_AVAILABLE				  	(10)
#define LED_OFF							  	(0)
#define LED_ON							  	(1)
#define LED_PROV						  	(2)

#define LONG_PRESS_TIME_TICKS   (32768 / 4)
#define VERY_LONG_PRESS_TIME_TICKS  (32768)

/* external signal definitions. these are used to signal button press events from GPIO interrupt handler to
 * application */
#define EXT_SIGNAL_PB0_SHORT_PRESS      0x01
#define EXT_SIGNAL_PB0_LONG_PRESS       0x02
#define EXT_SIGNAL_PB1_SHORT_PRESS      0x04
#define EXT_SIGNAL_PB1_LONG_PRESS       0x08
#define EXT_SIGNAL_PB0_VERY_LONG_PRESS  0x10
#define EXT_SIGNAL_PB1_VERY_LONG_PRESS  0x20

/** global variables */
static uint16 _primary_elem_index = 0xffff; /* For indexing elements of the node */
static uint16 _secondary_elem_index = 0xffff; /* For indexing elements of the node */
static uint16 _my_address = 0; /* Address of the Primary Element of the Node */
static uint8 num_connections = 0; /* number of active Bluetooth connections */
static uint8 conn_handle = 0xFF; /* handle of the last opened LE connection */
static uint8 trid = 0; /*transcation identifier*/
static uint8 init_done = 0;
static uint8 sprinkler_onoff = 0; /*current position of the sprinkler to be sent*/

/*Moisture reading from the adc*/
extern uint32_t soil_moisture_raw_value;
uint16_t moisture_plus_onoff_value = 0;
uint16_t soil_moisture_percentage_value = 0;

/*event scheduler variable*/
volatile uint8_t external_event_scheduler = 0;

/* button press timestamps for long/short press detection */
static uint32 pb0_press;
static uint32 pb1_press;

/* copy of transition delay parameter, needed for delayed on/off, delayed level requests */
uint32_t delayed_onoff_trans = 0;
uint32_t delayed_pri_level_trans = 0;

static int sprinkler_state_load(void);
static int sprinkler_state_store(void);
static void sprinkler_state_changed(void);
void gpioint(uint8_t);
void enable_button_interrupts(void);
void mesh_lib_generic_server_status_received(
		struct gecko_msg_mesh_generic_client_server_status_evt_t *evt);

/**
 * This is a callback function that is invoked each time a GPIO interrupt in one of the pushbutton
 * inputs occurs. Pin number is passed as parameter.
 *
 * Note: this function is called from ISR context and therefore it is not possible to call any BGAPI
 * functions directly. The button state change is signaled to the application using gecko_external_signal()
 * that will generate an event gecko_evt_system_external_signal_id which is then handled in the main loop.
 */
void gpioint(uint8_t pin)
{
	printf("GPIO interrupt handler\r\n");
	uint32_t t_diff;

	if (pin == BSP_BUTTON0_PIN)
	{
		if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0)
		{
			// PB0 pressed - record RTCC timestamp
			pb0_press = RTCC_CounterGet();
		}
		else
		{
			// PB0 released - check if it was short or long press
			t_diff = RTCC_CounterGet() - pb0_press;
			if (t_diff < LONG_PRESS_TIME_TICKS)
			{
				gecko_external_signal(EXT_SIGNAL_PB0_SHORT_PRESS);
			}
			else if (t_diff < VERY_LONG_PRESS_TIME_TICKS)
			{
				gecko_external_signal(EXT_SIGNAL_PB0_LONG_PRESS);
			}
			else
			{
				gecko_external_signal(EXT_SIGNAL_PB0_VERY_LONG_PRESS);
			}
		}
	}
	else if (pin == BSP_BUTTON1_PIN)
	{
		if (GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0)
		{
			// PB1 pressed - record RTCC timestamp
			pb1_press = RTCC_CounterGet();
		}
		else
		{
			// PB1 released - check if it was short or long press
			t_diff = RTCC_CounterGet() - pb1_press;
			if (t_diff < LONG_PRESS_TIME_TICKS)
			{
				gecko_external_signal(EXT_SIGNAL_PB1_SHORT_PRESS);
			}
			else if (t_diff < VERY_LONG_PRESS_TIME_TICKS)
			{
				gecko_external_signal(EXT_SIGNAL_PB1_LONG_PRESS);
			}
			else
			{
				gecko_external_signal(EXT_SIGNAL_PB1_VERY_LONG_PRESS);
			}
		}
	}
}

/**
 * Enable button interrupts for PB0, PB1. Both GPIOs are configured to trigger an interrupt on the
 * rising edge (button released).
 */
void enable_button_interrupts(void)
{

	GPIOINT_Init();

	/* configure interrupt for PB0 and PB1, both falling and rising edges */
	GPIO_ExtIntConfig(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, BSP_BUTTON0_PIN, true,
	true, true);
	GPIO_ExtIntConfig(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, BSP_BUTTON1_PIN, true,
	true, true);

	/* register the callback function that is invoked when interrupt occurs */
	GPIOINT_CallbackRegister(BSP_BUTTON0_PIN, gpioint);
	GPIOINT_CallbackRegister(BSP_BUTTON1_PIN, gpioint);
}

/**
 * Structure for Sprinkler
 */

static PACKSTRUCT(struct sprinkler_state
		{
			// On/Off Server state
			uint8_t onoff_current;
			uint8_t onoff_target;

			uint8_t onpowerup;

			// Primary Generic Level
			int16_t moisture_level_current;
			int16_t moisture_level_target;

		})
sprinkler_state;

static errorcode_t pri_level_response(uint16_t element_index,
		uint16_t client_addr, uint16_t appkey_index)
{
	struct mesh_generic_state current, target;

	current.kind = mesh_generic_state_level;
	current.level.level = sprinkler_state.moisture_level_current;

	target.kind = mesh_generic_state_level;
	target.level.level = sprinkler_state.moisture_level_target;

	return mesh_lib_generic_server_response(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
			element_index, client_addr, appkey_index, &current, &target, 0,
			0x00);
}

static errorcode_t pri_level_update(uint16_t element_index)
{
	struct mesh_generic_state current, target;

	current.kind = mesh_generic_state_level;
	current.level.level = sprinkler_state.moisture_level_current;

	target.kind = mesh_generic_state_level;
	target.level.level = sprinkler_state.moisture_level_target;

	return mesh_lib_generic_server_update(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
			element_index, &current, &target, 0);
}

static errorcode_t pri_level_update_and_publish(uint16_t element_index)
{
	errorcode_t e;

	e = pri_level_update(element_index);
	if (e == bg_err_success)
	{
		e = mesh_lib_generic_server_publish(MESH_GENERIC_LEVEL_SERVER_MODEL_ID,
				element_index, mesh_generic_state_level);
	}

	return e;
}

static void pri_level_request(uint16_t model_id, uint16_t element_index,
		uint16_t client_addr, uint16_t server_addr, uint16_t appkey_index,
		const struct mesh_generic_request *request, uint32_t transition_ms,
		uint16_t delay_ms, uint8_t request_flags)
{

	if (request->kind != mesh_generic_request_level)
	{
		return;
	}

	printf("pri_level_request: level=%d, \r\n", request->level);

	if (sprinkler_state.moisture_level_current == request->level)
	{
		printf("Request for current state received; no op\n");
	}
	else
	{
		printf("Setting pri_level to <%d>\r\n", request->level);

		if (transition_ms == 0 && delay_ms == 0)
		{ // Immediate change
			sprinkler_state.moisture_level_current = request->level;
			sprinkler_state.moisture_level_target = request->level;
		}
		sprinkler_state_changed();
	}

	if (request_flags & MESH_REQUEST_FLAG_RESPONSE_REQUIRED)
	{
		pri_level_response(element_index, client_addr, appkey_index);
	}
	else
	{
		pri_level_update(element_index);
	}
}

static void pri_level_change(uint16_t model_id, uint16_t element_index,
		const struct mesh_generic_state *current,
		const struct mesh_generic_state *target, uint32_t remaining_ms)
{
	if (sprinkler_state.moisture_level_current != current->level.level)
	{
		printf("pri_level_change: from %d to %d\r\n",
				sprinkler_state.moisture_level_current, current->level.level);
		sprinkler_state.moisture_level_current = current->level.level;
		sprinkler_state_changed();
	}
	else
	{
		printf("pri_level update -same value (%d)\r\n",
				sprinkler_state.moisture_level_current);
	}
}

/*
 * @brief this function sends the level request into the network. The level value sent
 * by the client is moisture value + sprinkler_onoff status
 *
 * @param [in] retrans: indicates the retransmission frequency, here the request is sent only once
 *
 * @return: void
 * */

void send_pri_level_request(int retrans)
{
	uint16 delay;
	uint16 resp;
	struct mesh_generic_request req;

	req.kind = mesh_generic_request_level;

	/*set the level value to moisture value + sprinkler_onoff status*/
	req.level = moisture_plus_onoff_value;

	if (retrans == 0)
	{
		trid++;
	}

	delay = 0;

	/*publish into the network*/
	resp = gecko_cmd_mesh_generic_client_publish(
	MESH_GENERIC_LEVEL_CLIENT_MODEL_ID, _primary_elem_index, trid, 0, delay, 0,
			req.kind, 2, (uint8*) &req.level)->result;

	if (resp)
	{
		printf("sending moisture data request faild,code %x\r\n", resp);
	}
	else
	{
		printf("moisture data request sent, trid = %u\r\n", trid);
	}
}

/*
 * @brief: this functions sends level request if user presses PB0 button
 * This function was implemented for testing purposes, in application this part
 * is commented.
 *
 * */
void handle_long_press(int button)
{
	printf("Sending Level data to proxy\r\n");

	/* send the request this request is send only once*/
	send_pri_level_request(0);
}

/*
 * @brief: this function is the hanlder for the staus update received from the client
 * The client receives the status update from the server only when rain is detected. Hence
 * this function disables LETIMER0 and ADC0 (for a specific period of time for eg 1 min for demo purpose)
 * as moisture sampling is not required while raining. Hence energy saving is achieved by disabling
 * LETIMER0 and ADC0.
 *
 * @param [in] : pointer to the structure of the event generic_client_server_status
 *
 * @return: void
 * */

void mesh_lib_generic_server_status_received(
		struct gecko_msg_mesh_generic_client_server_status_evt_t *evt)
{
	uint16 resp;
	struct mesh_generic_request req;
	req.kind = mesh_generic_request_level;

	/*Disable LETIMER0 and ADC0 as while raining moisture sampling from
	 * adc is not required*/
	LETIMER_Enable(LETIMER0, false);

	ADC0->CMD = ADC_CMD_SINGLESTOP_MASK;

	/*Set a timer, which when expires enables LETIMER0 and adc0 to check moisture level
	 * and if require starts sampling */
	gecko_cmd_hardware_set_soft_timer(60 * 32768,
			TIMER_ID_STOP_ADC_WHILE_RAINING, 1);

	if (evt->parameters.data[1] == 0)
	{
		sprinkler_state.onoff_current = SPRINKLER_STATE_OFF;
		sprinkler_state.onoff_target = SPRINKLER_STATE_OFF;
		sprinkler_state_store();

		LEDS_onoff(LED_OFF);
		DI_Print("Raining", LCD_ROW_SPRINKLER_STATUS);

		soil_moisture_percentage_value = 0xFF;

		soil_moisture_percentage_value |= (SPRINKLER_STATE_OFF
				<< SPRINKLER_STATE_SHIFT);

		req.level = soil_moisture_percentage_value;

		resp = gecko_cmd_mesh_generic_client_set(
		MESH_GENERIC_LEVEL_CLIENT_MODEL_ID, _primary_elem_index, 1, 0, trid, 0,
				0, 0, req.kind, 2, (uint8*) &req.level)->result;

		if (resp)
		{
			printf(
					"sending after rain data request to only proxy faild,code %x\r\n",
					resp);
		}
		else
		{
			printf(
					"sending after rain data request to only proxy sent, trid = %u\r\n",
					trid);
		}
	}

}

/*
 * @brief: this function converts the raw adc value into valibrated % value within the 0-100%
 * this function appends the sprinkler state as an 8th bit of the return value
 * 8th bit : 0 = sprinkler off
 * 			 1 = sprinkler on
 *
 * @param moisture_level: raw adc data
 *
 * @return value: soil moisture value with sprinkler on/off state
 */

uint16_t moisture_to_onoff_commnad(uint32_t moisture_level)
{
	char level[10] =
	{ "" };

	soil_moisture_percentage_value = get_soil_moisture_calibrated_value(
			moisture_level);

	/*update the structure values*/
	sprinkler_state.moisture_level_current = soil_moisture_percentage_value;
	sprinkler_state.moisture_level_target = soil_moisture_percentage_value;

	sprintf(level, "Moisture %d", soil_moisture_percentage_value);
	DI_Print(level, LCD_ROW_MOISTURE_LEVEL);

	/*if moisture % value is > 50 then Turn off the sprinkler else turn the sprinkler on*/
	if (soil_moisture_percentage_value > 50)
	{
		soil_moisture_percentage_value |= (SPRINKLER_STATE_OFF
				<< SPRINKLER_STATE_SHIFT);
		LEDS_onoff(LED_OFF);
		DI_Print("Sprinkler OFF", LCD_ROW_SPRINKLER_STATUS);

		/*update the onoff state and store into the PS*/
		sprinkler_state.onoff_current = LED_STATE_OFF;
		sprinkler_state.onoff_target = LED_STATE_OFF;
		sprinkler_state_store();
	}
	else
	{
		soil_moisture_percentage_value |= (SPRINKLER_STATE_ON
				<< SPRINKLER_STATE_SHIFT);
		LEDS_onoff(LED_ON);
		DI_Print("Sprinkler ON", LCD_ROW_SPRINKLER_STATUS);

		/*update the onoff state and store into the PS*/
		sprinkler_state.onoff_current = LED_STATE_ON;
		sprinkler_state.onoff_target = LED_STATE_ON;

		sprinkler_state_store();
	}

	return soil_moisture_percentage_value;
}

/*
 * @brief this function reads raw adc value and to convert the same into
 * percentage value and then %value + on_off command call the related functions.
 * And finally sends a level request to the network
 *
 * */

void get_soil_moisture_adc_value(void)
{
	/*take the Soil moisture value from the ADC fifo*/
	soil_moisture_raw_value = ADC_DataSingleGet(ADC0);

	printf("received soil moisture values is %lu \r\n",
			soil_moisture_raw_value);
	/*get the calibrated soil moisture value with the sprinkler state*/
	moisture_plus_onoff_value = moisture_to_onoff_commnad(
			soil_moisture_raw_value);

	/*send level request into the network*/
	send_pri_level_request(0);
}

/**
 * This function loads the saved light state from Persistent Storage and
 * copies the data in the global variable sprinkler_state
 */
static int sprinkler_state_load(void)
{
	struct gecko_msg_flash_ps_load_rsp_t* pLoad;

	pLoad = gecko_cmd_flash_ps_load(0x4004);

	if (pLoad->result)
	{
		memset(&sprinkler_state, 0, sizeof(struct sprinkler_state));

		return -1;
	}

	memcpy(&sprinkler_state, pLoad->value.data, pLoad->value.len);

	return 0;
}

/**
 * this function saves the current sprinkler state in Persistent Storage so that
 * the data is preserved over reboots and power cycles. The sprinkler state is hold
 * in a global variable sprinkler_state. a PS key with ID 0x4004 is used to store
 * the whole struct.
 */
static int sprinkler_state_store(void)
{
	struct gecko_msg_flash_ps_save_rsp_t* pSave;

	pSave = gecko_cmd_flash_ps_save(0x4004, sizeof(struct sprinkler_state),
			(const uint8*) &sprinkler_state);

	if (pSave->result)
	{
		printf("sprinkler_state_store(): PS save failed, code %x\r\n",
				pSave->result);
		return (-1);
	}

	return 0;
}

/**
 * this function is called each time the sprinkler state in RAM is changed. It sets up a soft timer
 * that will save the state in flash after small delay. The purpose is to reduce amount of unnecessary
 * flash writes.
 */
static void sprinkler_state_changed(void)
{
	gecko_cmd_hardware_set_soft_timer(TIMER_MS_2_TIMERTICK(5000),
	TIMER_ID_SAVE_STATE, 1);
}

/**
 * Initialization of the models supported by this node. This function registers callbacks for
 * each of the three supported models.
 */
static void init_models(void)
{
	mesh_lib_generic_server_register_handler(
	MESH_GENERIC_LEVEL_SERVER_MODEL_ID, 0, pri_level_request, pri_level_change);
}

/**
 * Friend node initialization. This is called at each boot if provisioning is already done.
 * Otherwise this function is called after provisioning is completed.
 */
void sprinkler_state_init(void)
{
	uint16 res;

	/* Initialize mesh lib */
	mesh_lib_init(malloc, free, 8);

	//Initialize Friend functionality
	printf("Friend mode initialization\r\n");
	res = gecko_cmd_mesh_friend_init()->result;
	if (res)
	{
		printf("Friend init failed 0x%x\r\n", res);
	}

	memset(&sprinkler_state, 0, sizeof(struct sprinkler_state));
	if (sprinkler_state_load() != 0)
	{
		printf("sprinkler_state_load() failed, using defaults\r\n");
		goto publish;
	}

	sprinkler_state.onpowerup = MESH_GENERIC_ON_POWER_UP_STATE_RESTORE;

	// Handle on power up behavior
	switch (sprinkler_state.onpowerup)
	{
	case MESH_GENERIC_ON_POWER_UP_STATE_RESTORE:
		printf("On power up state is RESTORE\r\n");
		if (sprinkler_state.onoff_current != sprinkler_state.onoff_target)
		{
			// update current state without any transition time
			if (sprinkler_state.onoff_target == MESH_GENERIC_ON_OFF_STATE_OFF)
			{
				LEDS_onoff(LED_OFF);
			}
			else
			{
				LEDS_onoff(LED_ON);
			}
			sprinkler_state.onoff_current = sprinkler_state.onoff_target;

		}
		else
		{
			printf("Keeping loaded state\r\n");
			if (sprinkler_state.onoff_current == MESH_GENERIC_ON_OFF_STATE_OFF)
			{
				LEDS_onoff(LED_OFF);
				DI_Print("Sprinkler OFF", LCD_ROW_SPRINKLER_STATUS);
			}
			else
			{
				LEDS_onoff(LED_ON);
				DI_Print("Sprinkler ON", LCD_ROW_SPRINKLER_STATUS);
			}
		}
		break;
	}

	publish: sprinkler_state_changed();
	init_models();
	//power_onoff_update_and_publish(_primary_elem_index);

	init_done = 1;
}

static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt);

/**
 * button initialization. Configure pushbuttons PB0,PB1
 * as inputs.
 */
static void button_init()
{
// configure pushbutton PB0 and PB1 as inputs, with pull-up enabled
	GPIO_PinModeSet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN, gpioModeInputPull, 1);
	GPIO_PinModeSet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN, gpioModeInputPull, 1);

	//GPIO as an input
	GPIO_PinModeSet(gpioPortF, 3, gpioModeDisabled, 0);
}

/**
 * Set device name in the GATT database. A unique name is generated using
 * the two last bytes from the Bluetooth address of this device. Name is also
 * displayed on the LCD.
 */
void set_device_name(bd_addr *pAddr)
{
	char name[20];
	uint16 res;

// create unique device name using the last two bytes of the Bluetooth address
	sprintf(name, "Friend node %x:%x", pAddr->addr[1], pAddr->addr[0]);

	printf("Device name: '%s'\r\n", name);

	res = gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0,
			strlen(name), (uint8 *) name)->result;
	if (res)
	{
		printf(
				"gecko_cmd_gatt_server_write_attribute_value() failed, code %x\r\n",
				res);
	}

// show device name on the LCD
	DI_Print(name, DI_ROW_NAME);
}

static void server_state_changed(
		struct gecko_msg_mesh_generic_server_state_changed_evt_t *pEvt)
{
	int i;

	printf("state changed: ");
	printf("model ID %4.4x, type %2.2x ", pEvt->model_id, pEvt->type);
	for (i = 0; i < pEvt->parameters.len; i++)
	{
		printf("%2.2x ", pEvt->parameters.data[i]);
	}
	printf("\r\n");
}

/**
 *  this function is called to initiate factory reset. Factory reset may be initiated
 *  by keeping one of the WSTK pushbuttons pressed during reboot. Factory reset is also
 *  performed if it is requested by the provisioner (event gecko_evt_mesh_node_reset_id)
 */
void initiate_factory_reset(void)
{
	printf("factory reset\r\n");
	DI_Print("\n***\nFACTORY RESET\n***", DI_ROW_STATUS);

	/* if connection is open then close it before rebooting */
	if (conn_handle != 0xFF)
	{
		gecko_cmd_le_connection_close(conn_handle);
	}

	/* perform a factory reset by erasing PS storage. This removes all the keys and other settings
	 that have been configured for this node */
	gecko_cmd_flash_ps_erase_all();
// reboot after a small delay
	gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}

int main()
{

	/*Initialize device*/
	initMcu();
	/* Initialize board*/
	initBoard();
	/*Initialize application*/
	initApp();

	gecko_stack_init(&config);
	gecko_bgapi_class_dfu_init();
	gecko_bgapi_class_system_init();
	gecko_bgapi_class_le_gap_init();
	gecko_bgapi_class_le_connection_init();
	//gecko_bgapi_class_gatt_init();
	gecko_bgapi_class_gatt_server_init();
	gecko_bgapi_class_endpoint_init();
	gecko_bgapi_class_hardware_init();
	gecko_bgapi_class_flash_init();
	gecko_bgapi_class_test_init();
	//gecko_bgapi_class_sm_init();
	//mesh_native_bgapi_init();
	gecko_bgapi_class_mesh_node_init();
	//gecko_bgapi_class_mesh_prov_init();
	gecko_bgapi_class_mesh_proxy_init();
	gecko_bgapi_class_mesh_proxy_server_init();
	//gecko_bgapi_class_mesh_proxy_client_init();
	gecko_bgapi_class_mesh_generic_client_init();
	gecko_bgapi_class_mesh_generic_server_init();
	//gecko_bgapi_class_mesh_vendor_model_init();
	//gecko_bgapi_class_mesh_health_client_init();
	//gecko_bgapi_class_mesh_health_server_init();
	//gecko_bgapi_class_mesh_test_init();
	//gecko_bgapi_class_mesh_lpn_init();
	gecko_bgapi_class_mesh_friend_init();

	gecko_initCoexHAL();

	RETARGET_SerialInit();

	/* initialize LEDs and buttons. Note: some radio boards share the same GPIO for button & LED.
	 * Initialization is done in this order so that default configuration will be "button" for those
	 * radio boards with shared pins. LEDS_init() is called later as needed to (re)initialize the LEDs
	 * */
	LEDS_modeset();
	button_init();

	/*Enable RTCC clock for button press timings*/
	CMU_ClockEnable(cmuClock_RTCC, true);

	/*LCD init*/
	DI_Init();

	/*setup timer0 for reading adc value after every 15 seconds*/
	setup_letimer0(ENERGY_MODE, ADC_SAMPLING_TIME);

	/*Setup adc for reading soil moisture values*/
	soil_moisture_adc0_setup();

	/*start letimer0*/
	LETIMER_Enable(LETIMER0, true);

	while (1)
	{
		struct gecko_cmd_packet *evt = gecko_wait_event();
		bool pass = mesh_bgapi_listener(evt);
		if (pass)
		{
			handle_gecko_event(BGLIB_MSG_ID(evt->header), evt);
		}
	}
}

/**
 * Handling of stack events. Both BLuetooth LE and Bluetooth mesh events are handled here.
 */
static void handle_gecko_event(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
	uint16_t result;
	char buf[30];

	struct gecko_msg_mesh_node_provisioning_failed_evt_t *prov_fail_evt;

	if (NULL == evt)
	{
		return;
	}

	switch (evt_id)
	{
	case gecko_evt_system_boot_id:
		// check pushbutton state at startup. If either PB0 or PB1 is held down then do factory reset
		if (GPIO_PinInGet(BSP_BUTTON0_PORT, BSP_BUTTON0_PIN) == 0
				|| GPIO_PinInGet(BSP_BUTTON1_PORT, BSP_BUTTON1_PIN) == 0)
		{
			initiate_factory_reset();
		}
		else
		{
			struct gecko_msg_system_get_bt_address_rsp_t *pAddr =
					gecko_cmd_system_get_bt_address();

			set_device_name(&pAddr->address);

			// Initialize Mesh stack in Node operation mode, wait for initialized event
			result = gecko_cmd_mesh_node_init_oob(0x00, 0x03, 0x03, 0x06, 0x00,
					0x06, 0x00)->result;
			if (result)
			{
				sprintf(buf, "init failed (0x%x)", result);
				DI_Print(buf, DI_ROW_STATUS);
			}

			/*// Capture the ADC value from the soil moisture sensor
			 gecko_cmd_hardware_set_soft_timer(32768*15, TIMER_ID_SOIL_MOISTURE_DATA, 0);*/

			// re-initialize LEDs (needed for those radio board that share same GPIO for button/LED)
			LEDS_modeset();
		}

		break;

	case gecko_evt_hardware_soft_timer_id:
		switch (evt->data.evt_hardware_soft_timer.handle)
		{
		case TIMER_ID_FACTORY_RESET:
			gecko_cmd_system_reset(0);
			break;

		case TIMER_ID_RESTART:
			gecko_cmd_system_reset(0);
			break;

		case TIMER_ID_PROVISIONING:
			LEDS_onoff(LED_PROV);
			break;


		case TIMER_ID_SAVE_STATE:
			sprinkler_state_store();
			break;

		case TIMER_ID_STOP_ADC_WHILE_RAINING:
			/*Enabling LETIMER0 and ADC0 to sample data after some time
			 * to check if its still raining or not and if requried starts sampling*/
			LETIMER_Enable(LETIMER0, true);
			ADC_Start(ADC0, adcStartSingle);
			break;

		default:
			break;
		}

		break;

	case gecko_evt_mesh_node_initialized_id:
		printf("Node initialized\r\n");

		gecko_cmd_mesh_generic_server_init();
		gecko_cmd_mesh_generic_client_init();

		struct gecko_msg_mesh_node_initialized_evt_t *pData =
				(struct gecko_msg_mesh_node_initialized_evt_t *) &(evt->data);

		if (pData->provisioned)
		{
			printf("Node is provisioned. address:%x, ivi:%ld\r\n",
					pData->address, pData->ivi);

			_my_address = pData->address;
			_primary_elem_index = 0;   // index of primary element is zero.
			_secondary_elem_index = 1; // index of secondary element is one.

			enable_button_interrupts();
			sprinkler_state_init();

			printf("Sprinkler initial state is <%s>\r\n",
					sprinkler_state.onoff_current ? "ON" : "OFF");
			DI_Print("Provisioned", DI_ROW_STATUS);
		}
		else
		{
			printf("Node is unprovisioned\r\n");
			DI_Print("Unprovisioned", DI_ROW_STATUS);

			printf("Starting unprovisioned beaconing...\r\n");
			gecko_cmd_mesh_node_start_unprov_beaconing(0x3); // enable ADV and GATT provisioning bearer
		}
		break;

	case gecko_evt_mesh_node_provisioning_started_id:
		printf("Started provisioning\r\n");
		DI_Print("Provisioning...", DI_ROW_STATUS);
		// start timer for blinking LEDs to indicate which node is being provisioned
		gecko_cmd_hardware_set_soft_timer(32768 / 4, TIMER_ID_PROVISIONING, 0);
		break;

	case gecko_evt_mesh_node_display_output_oob_id:
	{
		printf("evt: gecko_evt_mesh_node_display_output_oob\r\n");

		struct gecko_msg_mesh_node_display_output_oob_evt_t *pOOB =
				(struct gecko_msg_mesh_node_display_output_oob_evt_t *) &(evt->data);

		printf(
				"gecko_msg_mesh_node_display_output_oob_evt_t: action %d, size %d\r\n",
				pOOB->output_action, pOOB->output_size);

		snprintf(buf, 30, "OTP: %2.2x %2.2x %2.2x", pOOB->data.data[13],
				pOOB->data.data[14], pOOB->data.data[15]);
		DI_Print(buf, LCD_ROW_OOB_PASS);

		break;
	}

	case gecko_evt_mesh_node_provisioned_id:
		_primary_elem_index = 0;   // index of primary element is zero.
		_secondary_elem_index = 1; // index of secondary element is one.
		enable_button_interrupts();
		sprinkler_state_init();
		printf("Node provisioned with address=%x\r\n",
				evt->data.evt_mesh_node_provisioned.address);
		// stop LED blinking when provisioning complete
		gecko_cmd_hardware_set_soft_timer(0, TIMER_ID_PROVISIONING, 0);
		LEDS_onoff(LED_OFF);

		DI_Print("Provisioned", DI_ROW_STATUS);
		DI_Print("", LCD_ROW_OOB_PASS);

		break;

	case gecko_evt_mesh_node_provisioning_failed_id:
		prov_fail_evt =
				(struct gecko_msg_mesh_node_provisioning_failed_evt_t *) &(evt->data);
		printf("Provisioning failed, error code is %x\r\n",
				prov_fail_evt->result);
		DI_Print("Prov failed", DI_ROW_STATUS);
		/* start a one-shot timer that will trigger soft reset after small delay */
		gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_RESTART, 1);
		break;

	case gecko_evt_mesh_node_key_added_id:
		printf("Received new %s key with index %x\r\n",
				evt->data.evt_mesh_node_key_added.type == 0 ?
						"network" : "application",
				evt->data.evt_mesh_node_key_added.index);
		break;

	case gecko_evt_mesh_node_model_config_changed_id:
		printf("Model config changed\r\n");

		break;

	case gecko_evt_mesh_generic_server_client_request_id:
		printf("Request from Client is received\r\n");
		mesh_lib_generic_server_event_handler(evt);
		break;

	case gecko_evt_mesh_generic_server_state_changed_id:

		// uncomment following line to get debug prints for each server state changed event
		server_state_changed(
				&(evt->data.evt_mesh_generic_server_state_changed));

		// pass the server state changed event to mesh lib handler that will invoke
		// the callback functions registered by application
		mesh_lib_generic_server_event_handler(evt);

		break;

	case gecko_evt_mesh_generic_client_server_status_id:
		printf("server status event\r\n");

		/*handle the data recevied from the server to send Raining status in the network*/
		mesh_lib_generic_server_status_received(
				(struct gecko_msg_mesh_generic_client_server_status_evt_t*) evt);
		break;

	case gecko_evt_mesh_node_reset_id:
		printf("Node Reset\r\n");
		initiate_factory_reset();
		break;

	case gecko_evt_mesh_friend_friendship_established_id:
		printf("Friendship is established with LPN, lpn_address=%x\r\n",
				evt->data.evt_mesh_friend_friendship_established.lpn_address);

		DI_Print("FRIEND", DI_ROW_FRIEND);
		break;

	case gecko_evt_mesh_friend_friendship_terminated_id:
		printf("Friendship is terminated with LPN, reason=%x\r\n",
				evt->data.evt_mesh_friend_friendship_terminated.reason);
		DI_Print("NO LPN", DI_ROW_FRIEND);
		break;

	case gecko_evt_le_gap_adv_timeout_id:
		// adv timeout events silently discarded
		break;

	case gecko_evt_le_connection_opened_id:
		printf("Connection opened\r\n");
		num_connections++;
		conn_handle = evt->data.evt_le_connection_opened.connection;
		DI_Print("Connected", DI_ROW_CONNECTION);
		break;

	case gecko_evt_le_connection_parameters_id:
		printf("Conncetion Parameters\r\n");
		break;

	case gecko_evt_le_connection_closed_id:
		/* Check if need to boot to dfu mode */
		if (boot_to_dfu)
		{
			/* Enter to DFU OTA mode */
			gecko_cmd_system_reset(2);
		}

		printf("Connection is closed, reason 0x%x\r\n",
				evt->data.evt_le_connection_closed.reason);
		conn_handle = 0xFF;
		if (num_connections > 0)
		{
			if (--num_connections == 0)
			{
				DI_Print("", DI_ROW_CONNECTION);
			}
		}
		break;
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

	case gecko_evt_system_external_signal_id:
	{
		/*printf("external signal value =  %d\r\n",
		 evt->data.evt_system_external_signal.extsignals);

		 if ((evt->data.evt_system_external_signal.extsignals
		 & ADC_DATA_AVAILABLE) != 0)
		 {
		 Moisture sensor value from ADC is available
		 * read the value and based on the value decide to on/off the sprinkler

		 printf("ADC external event scheduler \r\n");
		 handle_long_press(0);

		 }*/

		if ((evt->data.evt_system_external_signal.extsignals
				& LETIMER0_INTERRUPT_EVENT) != 0)
		{
			/*read adc value*/
			get_soil_moisture_adc_value();
		}

		/*Plan B: Press PB0 for sending level state value*/
		if (evt->data.evt_system_external_signal.extsignals
				& EXT_SIGNAL_PB0_SHORT_PRESS)
		{
			printf("PB0 short press event\r\n");
			handle_long_press(0);
		}

	}
		break;

	default:
		//printf("unhandled evt: %8.8x class %2.2x method %2.2x\r\n", evt_id, (evt_id >> 16) & 0xFF, (evt_id >> 24) & 0xFF);
		break;
	}
}
