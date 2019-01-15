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

#define BUFFER_SIZE						(35)
#define DISCONNECTED					(0)
#define CONNECTED						(1)
#define DISCOVER_SERVICE				(2)
#define SERVICE_FOUND					(3)
#define DISCOVER_CHARACTERISTIC			(4)
#define CHARACTERISTIC_FOUND			(5)
#define ENABLE_NOTIFICATION				(6)

#define UINT32_TO_FLT(b)         (((float)((int32_t)(b) & 0x00FFFFFFU)) * (float)pow(10,((int32_t)(b) >> 24)))
//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void disable_pb0_ext_interrupt();
void enable_pb0_ext_interrupt();
void pb0_interrupt_handler(uint8_t);
