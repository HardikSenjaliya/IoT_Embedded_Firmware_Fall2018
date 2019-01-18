/*
 * dma.h
 *
 *  Created on: 19-Nov-2018
 *      Author: hardik
 */

#ifndef SOURCE_DMA_H_
#define SOURCE_DMA_H_

#include "stdbool.h"
#include "em_ldma.h"

void dma_setup(void);
void dma_init(void);
void dma_xfer_config(void);
void dma_xfer_descriptor(void);










#endif /* SOURCE_DMA_H_ */
