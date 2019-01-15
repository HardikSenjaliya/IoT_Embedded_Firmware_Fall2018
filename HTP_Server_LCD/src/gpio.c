//***********************************************************************************
// Include files
//***********************************************************************************
#include "gpio.h"

extern uint8_t scheduled_event;

//***********************************************************************************
// defined files
//***********************************************************************************

//***********************************************************************************
// global variables
//***********************************************************************************

//***********************************************************************************
// function prototypes
//***********************************************************************************

//***********************************************************************************
// functions
//***********************************************************************************

/**
 * @brief  This function intilizes GPIO pin and ports as per the requirement of the
 * applilcation
 * @param  void
 * @retval void
 */

void gpio_init(void)
{

	// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
	//GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, LED0_default);

	//// Set LED ports to be standard output drive with default off (cleared)
	GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
	//GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
	GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, LED1_default);

	//Set PB0 as an external interrupt source
	GPIOINT_Init();
	GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInput, 0);
	GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_INT_NUMBER, true, false, true);
	GPIOINT_CallbackRegister(PB0_pin, pb0_interrupt_handler);

	NVIC_EnableIRQ(GPIO_EVEN_IRQn);

}


/**
 * @brief  This function is a callback function of the button PB0 interrupt
 * whenever interrupts occours it sends and external BLE event.
 * @param  Pin: button Pb0 pin number
 * @retval void
 */

void pb0_interrupt_handler(uint8_t pin)
{

	if(pin == PB0_pin)
	{
		//scheduled_event |= GPIO_PB0_EVENT_FLAG;
		gecko_external_signal(GPIO_PB0_EVENT_FLAG);
	}
}

/*void GPIO_EVEN_IRQHandler(void)
{
	uint32_t gpio_int_flags = GPIO_IntGet();

	CORE_AtomicDisableIrq();

	GPIO_IntClear(gpio_int_flags);

	if(gpio_int_flags & PB0_INT_NUMBER)
	{
		GPIO_PinOutSet(LED1_port, LED1_pin);
		gecko_external_signal(GPIO_PB0_EVENT_FLAG);
	}

	CORE_AtomicEnableIrq();
}*/


