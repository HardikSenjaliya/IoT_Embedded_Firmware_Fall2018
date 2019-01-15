//***********************************************************************************
// Include files
//***********************************************************************************
#include "src/cmu.h"

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
// function definitions
//***********************************************************************************
/**
  * @brief  Set different Peripheral Clocks
  * @param  EMx: Energy Mode
  *
  * @retval no return value
  */

void cmu_init(Energy_Modes EMx)
{
/*	//Enable LETIMER0 Clock
	switch (EMx)
	{
	case 0:
	case 1:
	case 2:
		//Enable LETIMER0 Clock
		CMU_OscillatorEnable(cmuOsc_LFXO, true, true);
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
		break;
	case 3:
		//Enable LETIMER0 Clock
		CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO);
		break;
	default:
		CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);

	}*/

	//Enable Low Frequency Clock Tree for CORELE
	//CMU_ClockEnable(cmuClock_HFLE, true);

	CMU_ClockEnable(cmuClock_GPIO, true);

	//Enable Peripheral Clocks for LETIMER0
	CMU_ClockEnable(cmuClock_LETIMER0, true);

	//Enable TIMER0 Clock
	CMU_ClockEnable(cmuClock_TIMER0, true);

	//Enable I2C Clock
	CMU_ClockEnable(cmuClock_HFPER, true);
	CMU_ClockEnable(cmuClock_I2C0, true);


}

