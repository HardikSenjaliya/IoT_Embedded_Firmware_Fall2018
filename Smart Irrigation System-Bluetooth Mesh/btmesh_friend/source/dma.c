/*

 * dma.c
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 *
 *
 *  I was trying to implement adc reading from DMA to further
 *  reduce energy consumption.
 *
 */


#include "dma.h"

void dma_xfer_config(void)
{
	LDMA_TransferCfg_t xfer_config = {

			.ldmaReqSel = ldmaPeripheralSignal_ADC0_SINGLE,
			.ldmaCtrlSyncPrsClrOff = 0,
			.ldmaCtrlSyncPrsClrOff = 0,
			.ldmaCtrlSyncPrsClrOn = 0,
			.ldmaCtrlSyncPrsSetOff = 0,
			.ldmaCtrlSyncPrsSetOn = 0,
			.ldmaReqDis = false,
			.ldmaReqSel = false,
			.ldmaCfgArbSlots = ldmaCfgArbSlotsAs1,
			.ldmaCfgSrcIncSign = ldmaCfgSrcIncSignPos,
			.ldmaCfgDstIncSign = ldmaCfgDstIncSignPos,
			.ldmaLoopCnt = 2

	};
}

void dma_init(void)
{
	LDMA_Init_t ldma_init = {

			.ldmaInitCtrlNumFixed = 1,
			.ldmaInitCtrlSyncPrsClrEn = 0,
			.ldmaInitCtrlSyncPrsSetEn = 0,
			.ldmaInitIrqPriority = 3
	};

	LDMA_Init(&ldma_init);

}

void dma_setup(void)
{
	/*Initialize the DMA*/
	dma_init();

	dma_xfer_config();

	/*Start DMA transfer in the Basic Mode*/
}

