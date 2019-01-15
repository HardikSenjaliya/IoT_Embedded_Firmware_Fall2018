//***********************************************************************************
// Include files
//***********************************************************************************

#include "em_gpio.h"
#include "gpiointerrupt.h"
#include "native_gecko.h"
#include "em_core.h"
#include "stdbool.h"

//***********************************************************************************
// defined files
//***********************************************************************************

//I2C port and pin assignment
#define I2C0_Port                   (gpioPortC)
#define GPIO_PC_PIN_10				(10)
#define GPIO_PC_PIN_11              (11)
#define I2C0_SCL_LINE				(GPIO_PC_PIN_10)
#define I2C0_SDA_LINE				(GPIO_PC_PIN_11)

//Enable pin for humidity and temperature sensor
#define TEMP_HUMI_SENSOR_Port		(gpioPortD)
#define GPIO_PD_PIN_15				(15)
#define SENSOR_ENABLE_PIN			(GPIO_PD_PIN_15)

//LED0 pin
#define	LED0_port 					(gpioPortF)
#define GPIO_PF_PIN_4				(4)
#define LED0_pin 					(GPIO_PF_PIN_4)
#define LED0_default	            (false) 	// off

// LED1 pin is
#define LED1_port 					(gpioPortF)
#define GPIO_PF_PIN_5				(5)
#define LED1_pin 					(GPIO_PF_PIN_5)
#define LED1_default				(false)	// off


//GPIO Interrupt
#define GPIO_PB0_EVENT_FLAG			(0x04)
#define PB0_port					(gpioPortF)
#define GPIO_PF_PIN_6				(6)
#define PB0_pin						(GPIO_PF_PIN_6)
#define PB0_INT_NUMBER				(6)

//GPIO Interrupt
#define GPIO_PB1_EVENT_FLAG			(0x05)
#define PB1_port					(gpioPortF)
#define GPIO_PF_PIN_7				(7)
#define PB1_pin						(GPIO_PF_PIN_7)
#define PB1_INT_NUMBER				(7)

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_init(void);
