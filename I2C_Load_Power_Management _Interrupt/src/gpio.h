//***********************************************************************************
// Include files
//***********************************************************************************

#include "em_gpio.h"
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

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_init(void);

