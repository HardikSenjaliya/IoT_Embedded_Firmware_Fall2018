//***********************************************************************************
// Include files
//***********************************************************************************

#include <src/prescaler.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/cmu.h"
#include "letimer0.h"
#include "sleep.h"
#include "i2c0.h"
#include "load_power_management.h"
#include "timer.h"
#include "infrastructure.h"
//***********************************************************************************
// defined files
//***********************************************************************************

#define BLE_MIN_ADVRT_TIME				(250 * 1.6)
#define BLE_MAX_ADVRT_TIME				(250 * 1.6)
#define BLE_MIN_CONNE_INTERVAL			(75  / 1.25)
#define BLE_MAX_CONNE_INTERVAL			(75  / 1.25)
#define BLE_SLAVE_LATENCY				(3)
#define BLE_SUPERVISON_TIME				(800)
#define BUFFER_SIZE						(35)
//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void send_temperature_to_phone(double);
void set_tx_power(int8_t);
void disable_pb0_ext_interrupt();
